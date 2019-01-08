/*
 * This file is part of the libsigrokflow project.
 *
 * Copyright (C) 2018 Martin Ling <martin-sigrok@earth.li>
 * Copyright (C) 2018 Uwe Hermann <uwe@hermann-uwe.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <iostream>
#include <libsigrokflow/libsigrokflow.hpp>

namespace Srf
{

using namespace std;
using namespace std::placeholders;

static bool srf_initialized_ = false;

void init()
{
	if (srf_initialized_)
		throw runtime_error("libsigrokflow is already initialized");

#ifdef HAVE_LIBSIGROKCXX
	Gst::Plugin::register_static(GST_VERSION_MAJOR, GST_VERSION_MINOR,
			"sigrok_legacy_capture_device",
			"Wrapper for capture devices using legacy libsigrok APIs",
			sigc::ptr_fun(&LegacyCaptureDevice::register_element),
			"0.01", "GPL", "sigrok", "libsigrokflow", "http://sigrok.org");
	Gst::Plugin::register_static(GST_VERSION_MAJOR, GST_VERSION_MINOR,
			"sigrok_legacy_input",
			"Wrapper for inputs using legacy libsigrok APIs",
			sigc::ptr_fun(&LegacyInput::register_element),
			"0.01", "GPL", "sigrok", "libsigrokflow", "http://sigrok.org");
	Gst::Plugin::register_static(GST_VERSION_MAJOR, GST_VERSION_MINOR,
			"sigrok_legacy_output",
			"Wrapper for outputs using legacy libsigrok APIs",
			sigc::ptr_fun(&LegacyOutput::register_element),
			"0.01", "GPL", "sigrok", "libsigrokflow", "http://sigrok.org");
#endif
#ifdef HAVE_LIBSIGROKDECODE
	Gst::Plugin::register_static(GST_VERSION_MAJOR, GST_VERSION_MINOR,
			"sigrok_legacy_decoder",
			"Wrapper for protocol decoders using legacy libsigrokdecode APIs",
			sigc::ptr_fun(&LegacyDecoder::register_element),
			"0.01", "GPL", "sigrok", "libsigrokflow", "http://sigrok.org");
#endif

	srf_initialized_ = true;
}

void deinit()
{
	srf_initialized_ = false;
}

Sink::Sink(GstBaseSink *gobj) :
	Gst::BaseSink(gobj)
{
}

Device::Device(GstElement *gobj) :
	Gst::Element(gobj)
{
}

CaptureDevice::CaptureDevice(GstElement *gobj) :
	Device(gobj)
{
}

#ifdef HAVE_LIBSIGROKCXX
void LegacyCaptureDevice::class_init(Gst::ElementClass<LegacyCaptureDevice> *klass)
{
	klass->set_metadata("sigrok legacy capture device",
			"Source", "Wrapper for capture devices using legacy libsigrok APIs",
			"Martin Ling");

	klass->add_pad_template(Gst::PadTemplate::create(
			"src",
			Gst::PAD_SRC,
			Gst::PAD_ALWAYS,
			Gst::Caps::create_any()));
}

bool LegacyCaptureDevice::register_element(Glib::RefPtr<Gst::Plugin> plugin)
{
	Gst::ElementFactory::register_element(plugin, "sigrok_legacy_capture_device",
			0, Gst::register_mm_type<LegacyCaptureDevice>(
				"sigrok_legacy_capture_device"));
	return true;
}

LegacyCaptureDevice::LegacyCaptureDevice(GstElement *gobj) :
	Glib::ObjectBase(typeid(LegacyCaptureDevice)),
	CaptureDevice(gobj)
{
	add_pad(_src_pad = Gst::Pad::create(get_pad_template("src"), "src"));
}

Glib::RefPtr<LegacyCaptureDevice>LegacyCaptureDevice::create(
	shared_ptr<sigrok::HardwareDevice> libsigrok_device)
{
	auto element = Gst::ElementFactory::create_element("sigrok_legacy_capture_device");
	if (!element)
		throw runtime_error("Failed to create element - plugin not registered?");
	auto device = Glib::RefPtr<LegacyCaptureDevice>::cast_static(element);
	device->_libsigrok_device = libsigrok_device;
	return device;
}

shared_ptr<sigrok::HardwareDevice> LegacyCaptureDevice::libsigrok_device()
{
	return _libsigrok_device;
}

Gst::StateChangeReturn LegacyCaptureDevice::change_state_vfunc(Gst::StateChange transition)
{
	switch (transition)
	{
		case Gst::STATE_CHANGE_READY_TO_PAUSED:
			return Gst::StateChangeReturn::STATE_CHANGE_NO_PREROLL;
		case Gst::STATE_CHANGE_PAUSED_TO_PLAYING:
			_task = Gst::Task::create(std::bind(&LegacyCaptureDevice::_run, this));
			_task->set_lock(_mutex);
			_src_pad->set_active(true);
			_task->start();
			return Gst::STATE_CHANGE_SUCCESS;
		default:
			return Gst::STATE_CHANGE_SUCCESS;
	}
}

void LegacyCaptureDevice::_datafeed_callback(
	shared_ptr<sigrok::Device> device,
	shared_ptr<sigrok::Packet> packet)
{
	(void) device;
	switch (packet->type()->id()) {
		case SR_DF_LOGIC:
		{
			auto logic = static_pointer_cast<sigrok::Logic>(packet->payload());
			auto mem = Gst::Memory::create(
					Gst::MEMORY_FLAG_READONLY,
					logic->data_pointer(),
					logic->data_length(),
					0,
					logic->data_length());
			auto buf = Gst::Buffer::create();
			buf->append_memory(move(mem));
			_src_pad->push(move(buf));
			break;
		}
		case SR_DF_END:
			_session->stop();
			_src_pad->push_event(Gst::EventEos::create());
			break;
		default:
			break;
	}
}

void LegacyCaptureDevice::_run()
{
	_session = _libsigrok_device->driver()->parent()->create_session();
	_session->add_device(_libsigrok_device);
	_session->add_datafeed_callback(bind(&LegacyCaptureDevice::_datafeed_callback, this, _1, _2));
	_session->start();
	_session->run();
	_task->stop();
}

void LegacyInput::class_init(Gst::ElementClass<LegacyInput> *klass)
{
	klass->set_metadata("sigrok legacy input",
			"Transform", "Wrapper for inputs using legacy libsigrok APIs",
			"Martin Ling");

	klass->add_pad_template(Gst::PadTemplate::create(
			"sink",
			Gst::PAD_SINK,
			Gst::PAD_ALWAYS,
			Gst::Caps::create_any()));

	klass->add_pad_template(Gst::PadTemplate::create(
			"src",
			Gst::PAD_SRC,
			Gst::PAD_ALWAYS,
			Gst::Caps::create_any()));
}

bool LegacyInput::register_element(Glib::RefPtr<Gst::Plugin> plugin)
{
	Gst::ElementFactory::register_element(plugin, "sigrok_legacy_input",
			0, Gst::register_mm_type<LegacyInput>(
				"sigrok_legacy_input"));
	return true;
}

LegacyInput::LegacyInput(GstElement *gobj) :
	Glib::ObjectBase(typeid(LegacyInput)),
	Gst::Element(gobj)
{
	add_pad(_sink_pad = Gst::Pad::create(get_pad_template("sink"), "sink"));
	add_pad(_src_pad = Gst::Pad::create(get_pad_template("src"), "src"));
	_sink_pad->set_chain_function(sigc::mem_fun(*this, &LegacyInput::chain));
}

Glib::RefPtr<LegacyInput> LegacyInput::create(
	shared_ptr<sigrok::InputFormat> libsigrok_input_format,
	map<string, Glib::VariantBase> options)
{
	auto element = Gst::ElementFactory::create_element("sigrok_legacy_input");
	if (!element)
		throw runtime_error("Failed to create element - plugin not registered?");
	auto input = Glib::RefPtr<LegacyInput>::cast_static(element);
	input->_libsigrok_input_format = libsigrok_input_format;
	input->_options = options;
	return input;
}

bool LegacyInput::start_vfunc()
{
	_libsigrok_input = _libsigrok_input_format->create_input(_options);
	auto context = _libsigrok_input_format->parent();
	_session = context->create_session();
	_session->add_device(_libsigrok_input->device());
	_session->add_datafeed_callback(bind(&LegacyInput::_datafeed_callback, this, _1, _2));
	_session->start();
	return true;
}

void LegacyInput::_datafeed_callback(
	shared_ptr<sigrok::Device> device,
	shared_ptr<sigrok::Packet> packet)
{
	(void) device;
	switch (packet->type()->id()) {
		case SR_DF_LOGIC:
		{
			auto logic = static_pointer_cast<sigrok::Logic>(packet->payload());
			auto mem = Gst::Memory::create(
					Gst::MEMORY_FLAG_READONLY,
					logic->data_pointer(),
					logic->data_length(),
					0,
					logic->data_length());
			auto buf = Gst::Buffer::create();
			buf->append_memory(move(mem));
			_src_pad->push(move(buf));
			break;
		}
		case SR_DF_END:
			_session->stop();
			_src_pad->push_event(Gst::EventEos::create());
			break;
		default:
			break;
	}
}

Gst::FlowReturn LegacyInput::chain(const Glib::RefPtr<Gst::Pad> &,
                        const Glib::RefPtr<Gst::Buffer> &buf)
{
	Gst::MapInfo info;
	buf->map(info, Gst::MAP_READ);
	_libsigrok_input->send(info.get_data(), info.get_size());
	buf->unmap(info);
	return Gst::FLOW_OK;
}

bool LegacyInput::stop_vfunc()
{
	_libsigrok_input->end();
	return true;
}

void LegacyOutput::class_init(Gst::ElementClass<LegacyOutput> *klass)
{
	klass->set_metadata("sigrok legacy output",
			"Sink", "Wrapper for outputs using legacy libsigrok APIs",
			"Martin Ling");

	klass->add_pad_template(Gst::PadTemplate::create(
			"sink",
			Gst::PAD_SINK,
			Gst::PAD_ALWAYS,
			Gst::Caps::create_any()));
}

bool LegacyOutput::register_element(Glib::RefPtr<Gst::Plugin> plugin)
{
	Gst::ElementFactory::register_element(plugin, "sigrok_legacy_output",
			0, Gst::register_mm_type<LegacyOutput>(
				"sigrok_legacy_output"));
	return true;
}

LegacyOutput::LegacyOutput(GstBaseSink *gobj) :
	Glib::ObjectBase(typeid(LegacyOutput)),
	Sink(gobj)
{
}

Glib::RefPtr<LegacyOutput>LegacyOutput::create(
	shared_ptr<sigrok::OutputFormat> libsigrok_output_format,
	shared_ptr<sigrok::Device> libsigrok_device,
	map<string, Glib::VariantBase> options)
{
	auto element = Gst::ElementFactory::create_element("sigrok_legacy_output");
	if (!element)
		throw runtime_error("Failed to create element - plugin not registered?");
	auto output = Glib::RefPtr<LegacyOutput>::cast_static(element);
	output->_libsigrok_output_format = libsigrok_output_format;
	output->_libsigrok_device = libsigrok_device;
	output->_options = options;
	return output;
}

bool LegacyOutput::start_vfunc()
{
	_libsigrok_output = _libsigrok_output_format->create_output(
			_libsigrok_device, _options);
	return true;
}

Gst::FlowReturn LegacyOutput::render_vfunc(const Glib::RefPtr<Gst::Buffer> &buffer)
{
	Gst::MapInfo info;
	buffer->map(info, Gst::MAP_READ);
	auto context = _libsigrok_output_format->parent();
	auto packet = context->create_logic_packet(
			info.get_data(), info.get_size(), 2);
	auto result = _libsigrok_output->receive(packet);
	cout << result;
	buffer->unmap(info);
	return Gst::FLOW_OK;
}

bool LegacyOutput::stop_vfunc()
{
	auto context = _libsigrok_output_format->parent();
	auto end_packet = context->create_end_packet();
	auto result = _libsigrok_output->receive(end_packet);
	cout << result;
	return true;
}
#endif

#ifdef HAVE_LIBSIGROKDECODE
void LegacyDecoder::class_init(Gst::ElementClass<LegacyDecoder> *klass)
{
	klass->set_metadata("sigrok legacy decoder",
			"Sink", "Wrapper for protocol decoders using legacy libsigrokdecode APIs",
			"Uwe Hermann");

	klass->add_pad_template(Gst::PadTemplate::create(
			"sink",
			Gst::PAD_SINK,
			Gst::PAD_ALWAYS,
			Gst::Caps::create_any()));
}

bool LegacyDecoder::register_element(Glib::RefPtr<Gst::Plugin> plugin)
{
	Gst::ElementFactory::register_element(plugin, "sigrok_legacy_decoder",
			0, Gst::register_mm_type<LegacyDecoder>(
				"sigrok_legacy_decoder"));
	return true;
}

LegacyDecoder::LegacyDecoder(GstBaseSink *gobj) :
	Glib::ObjectBase(typeid(LegacyDecoder)),
	Sink(gobj)
{
}

Glib::RefPtr<LegacyDecoder>LegacyDecoder::create(
	struct srd_session *libsigrokdecode_session, uint64_t unitsize)
{
	auto element = Gst::ElementFactory::create_element("sigrok_legacy_decoder");
	if (!element)
		throw runtime_error("Failed to create element - plugin not registered?");
	auto decoder = Glib::RefPtr<LegacyDecoder>::cast_static(element);
	decoder->_session = libsigrokdecode_session;
	decoder->_unitsize = unitsize;
	return decoder;
}

struct srd_session *LegacyDecoder::libsigrokdecode_session()
{
	return _session;
}

Gst::FlowReturn LegacyDecoder::render_vfunc(const Glib::RefPtr<Gst::Buffer> &buffer)
{
	Gst::MapInfo info;
	buffer->map(info, Gst::MAP_READ);
	uint64_t num_samples = info.get_size() / _unitsize;
	srd_session_send(_session, _abs_ss, _abs_ss + num_samples,
		info.get_data(), info.get_size(), _unitsize);
	_abs_ss += num_samples;
	buffer->unmap(info);
	return Gst::FLOW_OK;
}

bool LegacyDecoder::start_vfunc()
{
	_abs_ss = 0;
	srd_session_start(_session);
	return true;
}

bool LegacyDecoder::stop_vfunc()
{
	srd_session_terminate_reset(_session);
	return true;
}
#endif

}
