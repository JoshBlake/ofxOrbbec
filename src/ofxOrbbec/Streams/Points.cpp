#include "Points.h"

namespace ofxOrbbec {
	namespace Streams {
		//----------

        template <class T>
        T fast_max(const T& left, const T& right)
        {
            return left > right ? left : right;
        }

        template <class T>
        T fast_abs(const T& val)
        {
            return val < 0 ? -val : val;
        }

		void Points::update() {
			TemplateBaseImage<astra::PointStream, astra::PointFrame, float>::update();
			const auto numVertices = this->pixels.getWidth() * this->pixels.getHeight();
			
			//safety check
			if (numVertices == 0) {
				return;
			}

			if (this->mesh.getNumVertices() != numVertices) {
				this->mesh.getVertices().resize(numVertices);
			}
			memcpy(this->mesh.getVerticesPointer(), this->pixels.getData(), numVertices * sizeof(ofVec3f));

			//stitch faces. Ignore jumps > 100mm
			const auto maxJump = 100.0f;
			this->mesh.getIndices().clear();
			const auto* vertices = (const ofVec3f*) this->pixels.getData();

            const int skip = 640 / 640;

            vector<ofIndexType> indices;
            indices.reserve((640/skip)*(480/skip)*3);

			if (this->pixels.size() >= 640 * 480) {
                for (int y = 1; y < 480; y += skip) {
                    for (int x = 1; x < 640; x += skip) {
						auto topLeftIndex = (x - 1) + (y - 1) * 640;
						auto topRightIndex = (x)+(y - 1) * 640;
						auto bottomLeftIndex = (x - 1) + (y) * 640;
						auto bottomRightIndex = (x)+(y) * 640;

						const auto& topLeft = vertices[topLeftIndex];
						const auto& topRight = vertices[topRightIndex];
						const auto& bottomLeft = vertices[bottomLeftIndex];
						const auto& bottomRight = vertices[bottomRightIndex];

						//top left triangle
						{
							auto jump = fast_max(
								fast_max(fast_abs(topLeft.z - topRight.z), fast_abs(bottomLeft.z - topRight.z)),
								fast_abs(bottomLeft.z - topRight.z));

							if (jump < maxJump) {
                                indices.emplace_back(topRightIndex);
                                indices.emplace_back(topLeftIndex);
                                indices.emplace_back(bottomLeftIndex);
							}
						}

						//bottom right triangle
						{
							auto jump = fast_max(
								fast_max(fast_abs(bottomRight.z - topRight.z), fast_abs(topRight.z - bottomLeft.z)),
								fast_abs(bottomLeft.z - bottomRight.z));

							if (jump < maxJump) {
                                indices.emplace_back(bottomRightIndex);
                                indices.emplace_back(topRightIndex);
                                indices.emplace_back(bottomLeftIndex);
							}
						}
					}
				}
			}
            this->mesh.addIndices(indices);
		}

		//----------
		const ofMesh & Points::getMesh() {
			return this->mesh;
		}
	}
}