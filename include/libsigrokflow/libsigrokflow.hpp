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

#ifndef LIBSIGROKFLOW_LIBSIGROKFLOW_HPP
#define LIBSIGROKFLOW_LIBSIGROKFLOW_HPP

/* Temporary workaround, will be dropped later. */
#define HAVE_LIBSIGROKCXX 1
#define HAVE_LIBSIGROKDECODE 1

#include <gstreamermm.h>
#include <gstreamermm/private/element_p.h>
#include <gstreamermm/private/basesink_p.h>
#ifdef HAVE_LIBSIGROKCXX
#include <libsigrokcxx/libsigrokcxx.hpp>
#endif
#ifdef HAVE_LIBSIGROKDECODE
#include <libsigrokdecode/libsigrokdecode.h>
#endif

#include <libsigrokflow/main.hpp>
#include <libsigrokflow/init.hpp>

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

class LegacyInput :
	public Gst::Element
{
public:
	/* Create from libsigrok input. */
	static Glib::RefPtr<LegacyInput> create(
		shared_ptr<sigrok::InputFormat> format,
		map<string, Glib::VariantBase> options = map<string, Glib::VariantBase>());

	/* Chain function (not an override). */
	Gst::FlowReturn chain(const Glib::RefPtr<Gst::Pad> &pad,
			const Glib::RefPtr<Gst::Buffer> &buf);

	/* Event function (not an override). */
	bool event(const Glib::RefPtr<Gst::Pad> &pad,
			Glib::RefPtr<Gst::Event> &event);

	/* Gst class init. */
	static void class_init(Gst::ElementClass<LegacyInput> *klass);

	/* Register class with plugin. */
	static bool register_element(Glib::RefPtr<Gst::Plugin> plugin);

	/* Constructor used by element factory. */
	explicit LegacyInput(GstElement *gobj);

private:
	shared_ptr<sigrok::InputFormat> libsigrok_input_format_;
	shared_ptr<sigrok::Input> libsigrok_input_;
	shared_ptr<sigrok::Session> session_;
	map<string, Glib::VariantBase> options_;
	Glib::RefPtr<Gst::Pad> sink_pad_;
	Glib::RefPtr<Gst::Pad> src_pad_;

	void datafeed_callback(shared_ptr<sigrok::Device> device,
			shared_ptr<sigrok::Packet> packet);
};

class LegacyOutput :
	public Sink
{
public:
	/* Create from libsigrok output object. */
	static Glib::RefPtr<LegacyOutput> create(
		shared_ptr<sigrok::OutputFormat> libsigrok_output_format,
		map<string, Glib::VariantBase> options = map<string, Glib::VariantBase>());

	/* Override start. */
	bool start_vfunc() override;

	/* Override render. */
	Gst::FlowReturn render_vfunc(const Glib::RefPtr<Gst::Buffer> &buffer) override;

	/* Override stop. */
	bool stop_vfunc() override;

	/* Gst class init. */
	static void class_init(Gst::ElementClass<LegacyOutput> *klass);

	/* Register class with plugin. */
	static bool register_element(Glib::RefPtr<Gst::Plugin> plugin);

	/* Constructor used by element factory. */
	explicit LegacyOutput(GstBaseSink *gobj);

private:
	shared_ptr<sigrok::OutputFormat> libsigrok_output_format_;
	shared_ptr<sigrok::UserDevice> libsigrok_device_;
	shared_ptr<sigrok::Output> libsigrok_output_;
	map<string, Glib::VariantBase> options_;
};
#endif

#ifdef HAVE_LIBSIGROKDECODE
class LegacyDecoder :
	public Sink
{
public:
	static Glib::RefPtr<LegacyDecoder> create(
		struct srd_session *libsigrokdecode_session, uint64_t unitsize);

	/* Retrieve libsigrokdecode session. */
	struct srd_session *libsigrokdecode_session();

	/* Override start. */
	bool start_vfunc() override;

	/* Override render. */
	Gst::FlowReturn render_vfunc(const Glib::RefPtr<Gst::Buffer> &buffer) override;

	/* Override stop. */
	bool stop_vfunc() override;

	/* Gst class init. */
	static void class_init(Gst::ElementClass<LegacyDecoder> *klass);

	/* Register class with plugin. */
	static bool register_element(Glib::RefPtr<Gst::Plugin> plugin);

	/* Constructor used by element factory. */
	explicit LegacyDecoder(GstBaseSink *gobj);

private:
	struct srd_session *session_;
	uint64_t abs_ss_;
	uint64_t unitsite_;
};
#endif

}
#endif
