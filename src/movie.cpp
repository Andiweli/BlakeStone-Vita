/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2019 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
*/


#include <cstring>

#include "an_codes.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_vh.h"
#include "id_vl.h"
#include "jm_vl.h"
#include "movie.h"
#include "bstone_archiver.h"
#include "bstone_endian.h"


namespace
{


template<typename T>
T read_unaligned(const void* const source)
{
	T value{};
	std::memcpy(&value, source, sizeof(value));
	return value;
}


} // namespace


class Movie
{
public:
	//
	// Plays an animation.
	//
	// Returns:
	//    - True if movie file was found and played.
	//    - False otherwise.
	//
	bool play(
		const MovieId movie_id,
		const std::uint8_t* const palette);


private:
	struct Descriptor
	{
		AssetsCRefString file_base_name_;
		std::int8_t repeat_count_;
		std::int8_t tick_delay_;
	}; // Descriptor


	struct AnimFrame
	{
		static constexpr auto class_size = 12;

		std::uint16_t code;
		std::int32_t block_num;
		std::int32_t recsize;
	}; // AnimFrame

	static_assert(AnimFrame::class_size == sizeof(AnimFrame), "Class size mismatch.");


	struct AnimChunk
	{
		static constexpr auto class_size = 6;

		std::uint16_t opt;
		std::uint16_t offset;
		std::uint16_t length;

		void endian()
		{
			if (bstone::Endian::is_little())
			{
				return;
			}

			bstone::Endian::little_i(opt);
			bstone::Endian::little_i(offset);
			bstone::Endian::little_i(length);
		}
	}; // AnimChunk

	static_assert(AnimChunk::class_size == sizeof(AnimChunk), "Class size mismatch.");


	static constexpr auto max_movies = 4;
	static constexpr auto max_buffer_size = 65'536;

	using Descriptors = std::array<Descriptor, max_movies>;
	using Buffer = std::vector<char>;


	enum class Flag
	{
		none,
		fill,
		skip,
	}; // Flag


	bstone::FileStream file_stream_;
	Buffer buffer_;

	int buffer_offset_{}; // Length of data loaded into buffer.
	char* buffer_ptr_{}; // Pointer to the current buffered frame.
	char* next_ptr_{}; // Pointer to the next buffered frame.
	AnimFrame current_frame_{};

	bool has_more_pages_{};
	Flag flag_{};
	bool is_exit_{};
	bool is_ever_faded_{};
	int repeat_count_{};
	ControlInfo control_info_{};
	const std::uint8_t* palette_{};

	bstone::ArchiverUPtr archiver_;


	const Descriptor& get_descriptor(
		const MovieId movie_id);

	void initialize(
		const Descriptor& descriptor,
		const std::uint8_t* const palette);

	void uninitialize();

	void validate_frame_size(
		const int record_size) const;

	void jm_draw_block(
		const int byte_offset,
		const char* const source,
		const int length);

	void show_frame(
		const char* frame_data,
		const int frame_size);

	bool load_buffer();

	bool get_frame();

	void handle_page(
		const Descriptor& descriptor);
}; // Movie


const Movie::Descriptor& Movie::get_descriptor(
	const MovieId movie_id)
{
	static const auto descriptors = Descriptors
	{{
		{Assets::get_intro_fmv_base_name(), 1, 3}, // intro
		{Assets::get_episode_6_fmv_base_name(), 1, 3}, // final
		{Assets::get_episode_2_4_fmv_base_name(), 1, 3}, // final_2
		{Assets::get_episode_3_5_fmv_base_name(), 1, 3}, // final_3
	}};

	return descriptors[movie_id];
}

void Movie::initialize(
	const Descriptor& descriptor,
	const std::uint8_t* const palette)
{
	repeat_count_ = descriptor.repeat_count_;
	flag_ = Flag::fill;
	buffer_offset_ = 0;
	buffer_ptr_ = nullptr;
	next_ptr_ = nullptr;
	current_frame_ = {};
	has_more_pages_ = true;
	is_exit_ = false;
	is_ever_faded_ = false;
	control_info_ = {};
	palette_ = palette;

	::JM_VGALinearFill(0, ::vga_ref_width * ::vga_ref_height, 0);
	::VL_FillPalette(0, 0, 0);

	buffer_.resize(max_buffer_size);
	archiver_ = bstone::ArchiverFactory::create();

	::IN_ClearKeysDown();
}

void Movie::uninitialize()
{
	archiver_ = nullptr;
	buffer_.clear();
	buffer_offset_ = 0;
	buffer_ptr_ = nullptr;
	next_ptr_ = nullptr;
	current_frame_ = {};
	file_stream_.close();
}

void Movie::validate_frame_size(
	const int record_size) const
{
	if (record_size < 0 ||
		record_size > (max_buffer_size - AnimFrame::class_size))
	{
		archiver_->throw_exception("Movie frame size out of range.");
	}
}

void Movie::jm_draw_block(
	const int byte_offset,
	const char* const source,
	const int length)
{
	const auto screen_size = ::vga_ref_width * ::vga_ref_height;

	if (!source ||
		byte_offset < 0 ||
		length < 0 ||
		byte_offset > screen_size ||
		length > (screen_size - byte_offset))
	{
		archiver_->throw_exception("Movie drawing block out of range.");
	}

	auto x = byte_offset % ::vga_ref_width;
	auto y = byte_offset / ::vga_ref_width;

	for (int i = 0; i < length; ++i)
	{
		::VL_Plot(x, y, static_cast<std::uint8_t>(source[i]));

		++x;

		if (x == ::vga_ref_width)
		{
			x = 0;
			++y;
		}
	}
}

void Movie::show_frame(
	const char* frame_data,
	const int frame_size)
{
	if (!frame_data || frame_size < 0)
	{
		archiver_->throw_exception("Invalid movie frame.");
	}

	auto remaining = frame_size;
	auto cursor = frame_data;

	while (true)
	{
		// Movie chunks are tightly packed. Their addresses are not guaranteed
		// to have natural alignment on ARM, so all integer fields are copied to
		// aligned local objects before they are accessed.
		if (remaining < static_cast<int>(sizeof(std::uint16_t)))
		{
			archiver_->throw_exception("Missing movie frame terminator.");
		}

		auto option = read_unaligned<std::uint16_t>(cursor);
		option = bstone::Endian::little(option);

		if (option == 0)
		{
			return;
		}

		if (remaining < AnimChunk::class_size)
		{
			archiver_->throw_exception("Truncated movie animation chunk.");
		}

		auto chunk = read_unaligned<AnimChunk>(cursor);
		chunk.endian();

		cursor += AnimChunk::class_size;
		remaining -= AnimChunk::class_size;

		if (chunk.length > remaining)
		{
			archiver_->throw_exception("Movie animation chunk exceeds frame.");
		}

		jm_draw_block(chunk.offset, cursor, chunk.length);

		cursor += chunk.length;
		remaining -= chunk.length;
	}
}

bool Movie::load_buffer()
{
	auto frame = buffer_.data();
	auto free_space = max_buffer_size;

	next_ptr_ = frame;
	buffer_ptr_ = frame;
	buffer_offset_ = 0;

	while (free_space > 0)
	{
		const auto chunk_start = file_stream_.get_position();

		auto block = AnimFrame{};
		block.code = archiver_->read_uint16();
		block.block_num = archiver_->read_int32();
		block.recsize = archiver_->read_int32();

		if (block.code == AN_END_OF_ANIM)
		{
			return false;
		}

		validate_frame_size(block.recsize);

		const auto total_size = AnimFrame::class_size + block.recsize;

		if (total_size > free_space)
		{
			file_stream_.set_position(chunk_start);
			break;
		}

		// Never assign through an AnimFrame pointer into the packed char buffer.
		// A previous variable-length frame can leave this address only 2-byte
		// aligned, which causes a Data Abort on PS Vita for the 32-bit fields.
		std::memcpy(frame, &block, AnimFrame::class_size);

		frame += AnimFrame::class_size;
		free_space -= AnimFrame::class_size;
		buffer_offset_ += AnimFrame::class_size;

		if (block.recsize > 0)
		{
			archiver_->read_char_array(frame, block.recsize);
			frame += block.recsize;
			free_space -= block.recsize;
			buffer_offset_ += block.recsize;
		}
	}

	return true;
}

bool Movie::get_frame()
{
	if (buffer_offset_ == 0)
	{
		if (!has_more_pages_)
		{
			return false;
		}

		has_more_pages_ = load_buffer();

		if (buffer_offset_ == 0)
		{
			return false;
		}
	}

	if (buffer_offset_ < AnimFrame::class_size || !next_ptr_)
	{
		archiver_->throw_exception("Truncated movie frame header.");
	}

	buffer_ptr_ = next_ptr_;

	const auto buffer_begin = buffer_.data();
	const auto buffer_end = buffer_begin + buffer_.size();

	if (buffer_ptr_ < buffer_begin ||
		buffer_ptr_ > (buffer_end - AnimFrame::class_size))
	{
		archiver_->throw_exception("Movie frame pointer out of range.");
	}

	// Packed frames are not guaranteed to be 4-byte aligned. Copy the header
	// into an aligned object before accessing its 32-bit fields.
	current_frame_ = read_unaligned<AnimFrame>(buffer_ptr_);

	validate_frame_size(current_frame_.recsize);

	const auto total_size = AnimFrame::class_size + current_frame_.recsize;

	if (total_size > buffer_offset_ ||
		buffer_ptr_ > (buffer_end - total_size))
	{
		archiver_->throw_exception("Movie frame payload out of range.");
	}

	buffer_offset_ -= total_size;
	next_ptr_ = buffer_ptr_ + total_size;

	return true;
}

void Movie::handle_page(
	const Descriptor& descriptor)
{
	const auto& block = current_frame_;
	auto frame = buffer_ptr_ + AnimFrame::class_size;
	auto frame_size = block.recsize;

	::IN_ReadControl(0, &control_info_);

	switch (block.code)
	{
	case AN_SOUND:
	{
		if (frame_size < static_cast<int>(sizeof(std::uint16_t)))
		{
			archiver_->throw_exception("Truncated movie sound command.");
		}

		auto sound_chunk = read_unaligned<std::uint16_t>(frame);
		sound_chunk = bstone::Endian::little(sound_chunk);

		::sd_play_player_sound(sound_chunk, bstone::ActorChannel::item);
		break;
	}

	case AN_FADE_IN_FRAME:
		::VL_FadeIn(0, 255, palette_, 30);
		is_ever_faded_ = true;
		::screenfaded = false;
		break;

	case AN_FADE_OUT_FRAME:
		VW_FadeOut();
		::screenfaded = true;
		break;

	case AN_PAUSE:
	{
		if (frame_size < static_cast<int>(sizeof(std::uint16_t)))
		{
			archiver_->throw_exception("Truncated movie pause command.");
		}

		auto vbls = read_unaligned<std::uint16_t>(frame);
		vbls = bstone::Endian::little(vbls);

		::IN_UserInput(vbls);
		::IN_ClearKeysDown();
		control_info_ = {};
		break;
	}

	case AN_PAGE:
	{
		if (flag_ == Flag::fill)
		{
			if (frame_size < 1)
			{
				archiver_->throw_exception("Truncated first movie page.");
			}

			flag_ = Flag::none;
			::JM_VGALinearFill(
				0,
				::vga_ref_width * ::vga_ref_height,
				static_cast<std::uint8_t>(*frame));

			++frame;
			--frame_size;
		}

		show_frame(frame, frame_size);
		::VL_RefreshScreen();

		if (TimeCount < static_cast<std::uint32_t>(descriptor.tick_delay_))
		{
			const auto min_wait_time = 0;
			const auto max_wait_time = 2 * TickBase;

			auto wait_time =
				descriptor.tick_delay_ - static_cast<int>(TimeCount);

			if (wait_time < min_wait_time)
			{
				wait_time = min_wait_time;
			}

			if (wait_time > max_wait_time)
			{
				wait_time = max_wait_time;
			}

			if (wait_time > 0)
			{
				wait_time *= 1000;
				wait_time /= TickBase;
				::sys_sleep_for(wait_time);
			}
		}
		else
		{
			::sys_sleep_for(1000 / TickBase);
		}

		::TimeCount = 0;

		if (!::screenfaded &&
			(control_info_.button0 ||
				control_info_.button1 ||
				::LastScan != ScanCode::sc_none))
		{
			is_exit_ = true;

			if (is_ever_faded_)
			{
				VW_FadeOut();
				::screenfaded = true;
			}
		}
		break;
	}

	case AN_END_OF_ANIM:
		is_exit_ = true;
		break;

	default:
		archiver_->throw_exception("Unrecognized movie animation code.");
	}
}

bool Movie::play(
	const MovieId movie_id,
	const std::uint8_t* const palette)
{
	const auto& descriptor = get_descriptor(movie_id);

	initialize(descriptor, palette);

	::ca_open_resource(descriptor.file_base_name_, file_stream_);

	if (!file_stream_.is_open())
	{
		uninitialize();
		return false;
	}

	try
	{
		archiver_->initialize(&file_stream_);

		while (repeat_count_ && !is_exit_)
		{
			while (!is_exit_)
			{
				if (!get_frame())
				{
					break;
				}

				handle_page(descriptor);
			}

			--repeat_count_;
			flag_ = Flag::skip;
		}
	}
	catch (const bstone::ArchiverException&)
	{
		// A malformed or truncated optional movie must never take down the Vita
		// process. Abort it and let the caller continue to the next screen.
		uninitialize();
		return false;
	}

	uninitialize();
	return true;
}


bool movie_play(
	const MovieId movie_id,
	const std::uint8_t* const palette)
{
	static auto movie = Movie{};

	return movie.play(movie_id, palette);
}
