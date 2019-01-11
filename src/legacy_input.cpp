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
#include <libsigrokflow/legacy_input.hpp>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <gstreamermm/private/element_p.h>

namespace Srf
{

using namespace std;
using namespace std::placeholders;

#ifdef HAVE_LIBSIGROKCXX
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
	add_pad(sink_pad_ = Gst::Pad::create(get_pad_template("sink"), "sink"));
	add_pad(src_pad_ = Gst::Pad::create(get_pad_template("src"), "src"));
	sink_pad_->set_chain_function(sigc::mem_fun(*this, &LegacyInput::chain));
	sink_pad_->set_event_function(sigc::mem_fun(*this, &LegacyInput::event));
}

Glib::RefPtr<LegacyInput> LegacyInput::create(
	shared_ptr<sigrok::InputFormat> libsigrok_input_format,
	map<string, Glib::VariantBase> options)
{
	auto element = Gst::ElementFactory::create_element("sigrok_legacy_input");
	if (!element)
		throw runtime_error("Failed to create element - plugin not registered?");
	auto input = Glib::RefPtr<LegacyInput>::cast_static(element);
	input->libsigrok_input_format_ = libsigrok_input_format;
	input->libsigrok_input_ = libsigrok_input_format->create_input(options);
	return input;
}

void LegacyInput::datafeed_callback(
	shared_ptr<sigrok::Device> device,
	shared_ptr<sigrok::Packet> packet)
{
	(void)device;

	switch (packet->type()->id()) {
	case SR_DF_LOGIC: {
		auto logic = static_pointer_cast<sigrok::Logic>(packet->payload());
		auto mem = Gst::Memory::create(
				Gst::MEMORY_FLAG_READONLY,
				logic->data_pointer(),
				logic->data_length(),
				0,
				logic->data_length());
		auto buf = Gst::Buffer::create();
		buf->append_memory(move(mem));
		src_pad_->push(move(buf));
		break;
	}
	case SR_DF_END:
		session_->stop();
		src_pad_->push_event(Gst::EventEos::create());
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
	libsigrok_input_->send(info.get_data(), info.get_size());
	auto device = libsigrok_input_->device();
	if (!session_ && device) {
		auto context = libsigrok_input_format_->parent();
		session_ = context->create_session();
		session_->add_device(device);
		session_->add_datafeed_callback(bind(&LegacyInput::datafeed_callback, this, _1, _2));
	}
	buf->unmap(info);

	return Gst::FLOW_OK;
}

bool LegacyInput::event(const Glib::RefPtr<Gst::Pad>&pad, Glib::RefPtr<Gst::Event>&event)
{
	(void)pad;

	if (event->get_event_type() == Gst::EVENT_EOS) {
		libsigrok_input_->end();
		session_->stop();
	}

	return true;
}
#endif

}
