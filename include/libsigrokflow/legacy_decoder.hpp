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

#ifndef LIBSIGROKFLOW_LEGACY_DECODER_HPP
#define LIBSIGROKFLOW_LEGACY_DECODER_HPP

/* Temporary workaround, will be dropped later. */
#define HAVE_LIBSIGROKDECODE 1

#include <gstreamermm.h>
#ifdef HAVE_LIBSIGROKDECODE
#include <libsigrokdecode/libsigrokdecode.h>
#endif
#include <libsigrokflow/main.hpp>

namespace Srf
{

using std::shared_ptr;

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
	uint64_t unitsize_;
};
#endif

}
#endif
