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

#ifndef LIBSIGROKFLOW_LEGACY_CAPTURE_DEVICE_HPP
#define LIBSIGROKFLOW_LEGACY_CAPTURE_DEVICE_HPP

/* Temporary workaround, will be dropped later. */
#define HAVE_LIBSIGROKCXX 1

#include <gstreamermm.h>
#ifdef HAVE_LIBSIGROKCXX
#include <libsigrokcxx/libsigrokcxx.hpp>
#endif
#include <libsigrokflow/main.hpp>

namespace Srf
{

using namespace std;

#ifdef HAVE_LIBSIGROKCXX
class LegacyCaptureDevice :
	public CaptureDevice
{
public:
	/* Create from libsigrok device object. */
	static Glib::RefPtr<LegacyCaptureDevice> create(
		shared_ptr<sigrok::HardwareDevice> libsigrok_device);

	/* Retrieve libsigrok device object. */
	shared_ptr<sigrok::HardwareDevice> libsigrok_device();

	/* Override state change. */
	Gst::StateChangeReturn change_state_vfunc(Gst::StateChange transition) override;

	/* Gst class init. */
	static void class_init(Gst::ElementClass<LegacyCaptureDevice> *klass);

	/* Register class with plugin. */
	static bool register_element(Glib::RefPtr<Gst::Plugin> plugin);

	/* Constructor used by element factory. */
	explicit LegacyCaptureDevice(GstElement *gobj);

private:
	shared_ptr<sigrok::HardwareDevice> libsigrok_device_;
	Glib::RefPtr<Gst::Pad> src_pad_;
	Glib::Threads::RecMutex mutex_;
	Glib::RefPtr<Gst::Task> task_;
	shared_ptr<sigrok::Session> session_;

	void datafeed_callback(shared_ptr<sigrok::Device> device,
			shared_ptr<sigrok::Packet> packet);
	void run();
};
#endif

}
#endif
