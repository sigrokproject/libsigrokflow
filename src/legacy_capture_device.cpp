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
#include <libsigrokflow/legacy_capture_device.hpp>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>
#include <gstreamermm/private/element_p.h>

namespace Srf
{

using namespace std;
using namespace std::placeholders;

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
	add_pad(src_pad_ = Gst::Pad::create(get_pad_template("src"), "src"));
}

Glib::RefPtr<LegacyCaptureDevice>LegacyCaptureDevice::create(
	shared_ptr<sigrok::HardwareDevice> libsigrok_device)
{
	auto element = Gst::ElementFactory::create_element("sigrok_legacy_capture_device");
	if (!element)
		throw runtime_error("Failed to create element - plugin not registered?");
	auto device = Glib::RefPtr<LegacyCaptureDevice>::cast_static(element);
	device->libsigrok_device_ = libsigrok_device;

	return device;
}

shared_ptr<sigrok::HardwareDevice> LegacyCaptureDevice::libsigrok_device()
{
	return libsigrok_device_;
}

Gst::StateChangeReturn LegacyCaptureDevice::change_state_vfunc(Gst::StateChange transition)
{
	switch (transition) {
	case Gst::STATE_CHANGE_READY_TO_PAUSED:
		return Gst::StateChangeReturn::STATE_CHANGE_NO_PREROLL;
	case Gst::STATE_CHANGE_PAUSED_TO_PLAYING:
		task_ = Gst::Task::create(std::bind(&LegacyCaptureDevice::run, this));
		task_->set_lock(mutex_);
		src_pad_->set_active(true);
		task_->start();
		return Gst::STATE_CHANGE_SUCCESS;
	default:
		return Gst::STATE_CHANGE_SUCCESS;
	}
}

void LegacyCaptureDevice::datafeed_callback(
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

void LegacyCaptureDevice::run()
{
	session_ = libsigrok_device_->driver()->parent()->create_session();
	session_->add_device(libsigrok_device_);
	session_->add_datafeed_callback(bind(&LegacyCaptureDevice::datafeed_callback, this, _1, _2));
	session_->start();
	session_->run();
	task_->stop();
}
#endif

}
