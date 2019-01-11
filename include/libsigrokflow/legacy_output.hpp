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

#ifndef LIBSIGROKFLOW_LEGACY_OUTPUT_HPP
#define LIBSIGROKFLOW_LEGACY_OUTPUT_HPP

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

}
#endif
