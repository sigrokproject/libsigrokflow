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
#include <libsigrokflow/legacy_decoder.hpp>
#include <stdexcept>
#include <gstreamermm/private/basesink_p.h>

namespace Srf
{

using std::runtime_error;

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
	decoder->session_ = libsigrokdecode_session;
	decoder->unitsite_ = unitsize;

	return decoder;
}

struct srd_session *LegacyDecoder::libsigrokdecode_session()
{
	return session_;
}

Gst::FlowReturn LegacyDecoder::render_vfunc(const Glib::RefPtr<Gst::Buffer> &buffer)
{
	Gst::MapInfo info;
	buffer->map(info, Gst::MAP_READ);
	uint64_t num_samples = info.get_size() / unitsite_;
	srd_session_send(session_, abs_ss_, abs_ss_ + num_samples,
		info.get_data(), info.get_size(), unitsite_);
	abs_ss_ += num_samples;
	buffer->unmap(info);

	return Gst::FLOW_OK;
}

bool LegacyDecoder::start_vfunc()
{
	abs_ss_ = 0;
	srd_session_start(session_);

	return true;
}

bool LegacyDecoder::stop_vfunc()
{
	srd_session_terminate_reset(session_);

	return true;
}
#endif

}
