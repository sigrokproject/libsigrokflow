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

#ifdef HAVE_LIBSIGROKCXX
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
	map<string, Glib::VariantBase> options)
{
	auto element = Gst::ElementFactory::create_element("sigrok_legacy_output");
	if (!element)
		throw runtime_error("Failed to create element - plugin not registered?");
	auto output = Glib::RefPtr<LegacyOutput>::cast_static(element);
	output->libsigrok_output_format_ = libsigrok_output_format;
	auto context = libsigrok_output_format->parent();
	output->libsigrok_device_ = context->create_user_device("Vendor", "Model", "Version");
	for (int i = 0; i < 8; ++i) {
		gchar *name = g_strdup_printf("D%d", i);
		output->libsigrok_device_->add_channel(i, sigrok::ChannelType::LOGIC, name);
		g_free(name);
	}
	output->options_ = options;

	return output;
}

bool LegacyOutput::start_vfunc()
{
	libsigrok_output_ = libsigrok_output_format_->create_output(
			libsigrok_device_, options_);

	return true;
}

Gst::FlowReturn LegacyOutput::render_vfunc(const Glib::RefPtr<Gst::Buffer> &buffer)
{
	Gst::MapInfo info;
	buffer->map(info, Gst::MAP_READ);
	auto context = libsigrok_output_format_->parent();
	auto packet = context->create_logic_packet(
			info.get_data(), info.get_size(), 2);
	auto result = libsigrok_output_->receive(packet);
	cout << result;
	buffer->unmap(info);

	return Gst::FLOW_OK;
}

bool LegacyOutput::stop_vfunc()
{
	auto context = libsigrok_output_format_->parent();
	auto end_packet = context->create_end_packet();
	auto result = libsigrok_output_->receive(end_packet);
	cout << result;

	return true;
}
#endif

}
