#include <iostream>
#include <optional>
#include <chrono>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/rotate_vector.hpp>

void clear() {
	//std::cout << "\x1B[2J\x1B[H";
	system("clear");
}

float donut_sdf(glm::fvec3 p, double time) {
	// Rotate 30 degrees per second.
	auto r = glm::rotateX(p, glm::radians(30.f * static_cast<float>(time)));
	r = glm::rotateZ(r, glm::radians(70.f * static_cast<float>(time)));

	static constexpr auto t = glm::fvec2(0.25, 0.05);
	auto xz = glm::fvec2(r.x, r.z);
	auto q = glm::fvec2(length(xz) - t.x, r.y);
	return length(q) - t.y;
}

std::optional<glm::fvec3> march(glm::fvec3 initial_pos, glm::fvec3 dir, double time) {
	dir = glm::normalize(dir);
	glm::fvec3 pos = initial_pos;
	float dist;
	while (!glm::epsilonEqual(0.f, (dist = donut_sdf(pos, time)), std::numeric_limits<float>::epsilon())) {
		if (dist < 1E-5)
			break;

		pos += dir * dist;

		if (glm::distance(initial_pos, pos) >= 10.f)
			return std::nullopt;
	}
	return pos;
}

int main() {
	static constexpr auto screen_size = glm::i32vec2(32, 64);
	static constexpr auto camera_pos = glm::fvec3(0.f, 0.f, 1.f);
	static constexpr auto camera_scale = 0.4f;

	auto time_begin = std::chrono::high_resolution_clock::now();
	while (true) {
		auto time_now = std::chrono::high_resolution_clock::now();
		using f_ms = std::chrono::duration<double, std::milli>;
		auto dt = std::chrono::duration_cast<f_ms>(time_now - time_begin).count() / 1000.;

		for (std::int32_t i = 0; i < screen_size.x; ++i) {
			for (std::int32_t j = 0; j < screen_size.y; ++j) {
				auto offset = glm::fvec2(i * 2 - screen_size.x, j * 2 - screen_size.y) / glm::fvec2(screen_size);
				auto pos = camera_pos + glm::fvec3(offset * camera_scale, 0.f);
				auto hit = march(pos, glm::fvec3(0.f, 0.f, -1.f), dt);
				if (!hit) {
					std::cout << ' ';
					continue;
				}

				std::cout << 'x';
			}
			std::cout << '\n';
		}
		clear();
	}
	return 0;
}
