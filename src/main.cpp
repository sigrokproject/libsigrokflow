/*
 * This file is part of the libsigrokflow project.
 *
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
#include <libsigrokflow/libsigrokflow.hpp>

#include <iostream>

namespace Srf
{

using namespace std;
using namespace std::placeholders;

void init()
{
}

auto src_template = Gst::PadTemplate::create("src",
		Gst::PAD_SRC,
		Gst::PAD_ALWAYS,
		Gst::Caps::create_any());

LegacyCaptureDevice::LegacyCaptureDevice(shared_ptr<sigrok::HardwareDevice> device) :
	_device(device), _src_pad(Gst::Pad::create(src_template))
{
	add_pad(_src_pad);
}

shared_ptr<sigrok::HardwareDevice> LegacyCaptureDevice::libsigrok_device()
{
	return _device;
}

Gst::StateChangeReturn LegacyCaptureDevice::change_state_vfunc(Gst::StateChange transition)
{
	switch (transition)
	{
		case Gst::STATE_CHANGE_READY_TO_PAUSED:
			return Gst::StateChangeReturn::STATE_CHANGE_NO_PREROLL;
		case Gst::STATE_CHANGE_PAUSED_TO_PLAYING:
			_device->open();
			_device->config_set(sigrok::ConfigKey::LIMIT_SAMPLES,
					Glib::Variant<int>::create(10));
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
	_session = _device->driver()->parent()->create_session();
	_session->add_device(_device);
	_session->add_datafeed_callback(bind(&LegacyCaptureDevice::_datafeed_callback, this, _1, _2));
	_session->start();
	_session->run();
	_task->stop();
}

}
