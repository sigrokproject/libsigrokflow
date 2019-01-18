/*
 * This file is part of the libsigrokflow project.
 *
 * Copyright (C) 2019 Uwe Hermann <uwe@hermann-uwe.de>
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

#include <catch2/catch.hpp>
#include <libsigrokflow/libsigrokflow.hpp>
#include <libsigrokdecode/libsigrokdecode.h>

TEST_CASE("init", "[init]")
{
	REQUIRE_NOTHROW(Gst::init());
	REQUIRE_NOTHROW(Srf::init());

	srd_init(nullptr);

	struct srd_session *session;
	srd_session_new(&session);

	SECTION("Srf::LegacyDecoder::create() with valid session/unitsize should work") {
		const uint64_t unitsize = 1;
		auto decoder = Srf::LegacyDecoder::create(session, unitsize);

		CHECK(decoder);
		CHECK(decoder->libsigrokdecode_session() == session);
		CHECK(decoder->unitsize() == unitsize);
	}

	srd_exit();

	REQUIRE_NOTHROW(Srf::deinit());
	REQUIRE_NOTHROW(Gst::deinit());
}
