/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * file_sink.cpp - File Sink
 */

#include <assert.h>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string.h>
#include <unistd.h>

#include <libcamera/camera.h>

#include "image.h"

#include "file_sink.h"

using namespace libcamera;

FileSink::FileSink([[maybe_unused]] const libcamera::Camera *camera,
		   const std::map<const libcamera::Stream *, std::string> &streamNames,
		   const std::string &pattern)
	:
#ifdef HAVE_TIFF
	  camera_(camera),
#endif
	  streamNames_(streamNames), pattern_(pattern)
{
}

FileSink::~FileSink()
{
}

int FileSink::configure(const libcamera::CameraConfiguration &config)
{
	int ret = FrameSink::configure(config);
	if (ret < 0)
		return ret;

	return 0;
}

void FileSink::mapBuffer(FrameBuffer *buffer)
{
	std::unique_ptr<Image> image =
		Image::fromFrameBuffer(buffer, Image::MapMode::ReadOnly);
	assert(image != nullptr);

	mappedBuffers_[buffer] = std::move(image);
}

bool FileSink::processRequest(Request *request)
{
	// std::cout<<"\nInside Process Request FileSink"<<std::endl;
	for (auto [stream, buffer] : request->buffers())
		{
			// std::cout<<"\nBefore for loop"<<std::endl;
			writeBuffer(stream, buffer, request->metadata());
			// std::cout<<"\nAfter for loop"<<std::endl;
			}

	// std::cout<<"Leaving Process Request FileSink"<<std::endl;
	return true;
}

void FileSink::writeBuffer(const Stream *stream, FrameBuffer *buffer,
			   [[maybe_unused]] const ControlList &metadata)
{
	// std::cout<<"\nStart writeBuffer"<<std::endl;
	std::string filename;
	size_t pos;
	int fd, ret = 0;

	// std::cout<<"\nFile name = "<<filename<<std::endl;
	if (!pattern_.empty())
		filename = pattern_;

#ifdef HAVE_TIFF
	bool dng = filename.find(".dng", filename.size() - 4) != std::string::npos;
#endif /* HAVE_TIFF */

	if (filename.empty() || filename.back() == '/')
		filename += "frame-#.bin";

	// std::cout<<"\nFile name = "<<filename<<std::endl;
	pos = filename.find_first_of('#');
	if (pos != std::string::npos) {
		std::stringstream ss;
		ss << streamNames_[stream] << "-" << std::setw(6)
		   << std::setfill('0') << buffer->metadata().sequence;
		filename.replace(pos, 1, ss.str());
	}

	// std::cout<<"\nFile name = "<<filename<<std::endl;
	Image *image = mappedBuffers_[buffer].get();
	// std::cout<<"\nDefine image"<<std::endl;

#ifdef HAVE_TIFF
	if (dng) {
		ret = DNGWriter::write(filename.c_str(), camera_,
				       stream->configuration(), metadata,
				       buffer, image->data(0).data());
		if (ret < 0)
			std::cerr << "failed to write DNG file `" << filename
				  << "'" << std::endl;

		return;
	}
#endif /* HAVE_TIFF */

	fd = open(filename.c_str(), O_CREAT | O_WRONLY |
		  (pos == std::string::npos ? O_APPEND : O_TRUNC),
		  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (fd == -1) {
		ret = -errno;
		std::cerr << "failed to open file " << filename << ": "
			  << strerror(-ret) << std::endl;
		return;
	}
	// std::cout<<"\nFile opened"<<std::endl;

	for (unsigned int i = 0; i < buffer->planes().size(); ++i) {
		// std::cout<<"\ninside for loop of planes"<<std::endl;
		/*
		 * This was formerly a local "const FrameMetadata::Plane &"
		 * however this causes a false positive warning for dangling
		 * references on gcc 13.
		 */
		const unsigned int bytesused = buffer->metadata().planes()[i].bytesused;
		// std::cout<<"\nBytesused = "<<bytesused<<std::endl;

		Span<uint8_t> data = image->data(i);
		// std::cout<<"\nData obtained "<<std::endl;
		const unsigned int length = std::min<unsigned int>(bytesused, data.size());

		if (bytesused > data.size())
			std::cerr << "payload size " << bytesused
				  << " larger than plane size " << data.size()
				  << std::endl;

		ret = ::write(fd, data.data(), length);
		if (ret < 0) {
			ret = -errno;
			std::cerr << "write error: " << strerror(-ret)
				  << std::endl;
			break;
		} else if (ret != (int)length) {
			std::cerr << "write error: only " << ret
				  << " bytes written instead of "
				  << length << std::endl;
			break;
		}
	}

	close(fd);
}
