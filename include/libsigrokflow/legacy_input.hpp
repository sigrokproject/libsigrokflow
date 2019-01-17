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

#ifndef LIBSIGROKFLOW_LEGACY_INPUT_HPP
#define LIBSIGROKFLOW_LEGACY_INPUT_HPP

/* Temporary workaround, will be dropped later. */
#define HAVE_LIBSIGROKCXX 1

#include <gstreamermm.h>
#ifdef HAVE_LIBSIGROKCXX
#include <libsigrokcxx/libsigrokcxx.hpp>
#endif
#include <libsigrokflow/main.hpp>

namespace Srf
{

using std::map;
using std::shared_ptr;
using std::string;

#ifdef HAVE_LIBSIGROKCXX
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
#endif

}
#endif
