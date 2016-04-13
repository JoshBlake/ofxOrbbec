#include "Infrared.h"

namespace ofxOrbbec {
	namespace Streams {
		//----------
		void Infrared::init(astra::StreamReader & streamReader) {
			this->stream = make_unique<astra::InfraredStream>(streamReader.stream<astra::InfraredStream>());

			{
				astra::ImageStreamMode irMode;
				irMode.set_width(640);
				irMode.set_height(480);
				irMode.set_pixel_format(astra_pixel_formats::ASTRA_PIXEL_FORMAT_GRAY16);
				irMode.set_fps(30);
				this->stream->set_mode(irMode);
			}

			this->stream->start();
		}
	}
}