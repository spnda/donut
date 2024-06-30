#include <iostream>
#include <optional>
#include <chrono>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vec_swizzle.hpp>

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

// See https://iquilezles.org/articles/normalsSDF/
glm::fvec3 calcNormal(glm::fvec3 p, double time) {
	const float h = 0.0001f;
	const auto k = glm::fvec2(1, -1);
	auto n =
			glm::xyy(k) * donut_sdf(p + glm::xyy(k) * h, time) +
			glm::yyx(k) * donut_sdf(p + glm::yyx(k) * h, time) +
			glm::yxy(k) * donut_sdf(p + glm::yxy(k) * h, time) +
			glm::xxx(k) * donut_sdf(p + glm::xxx(k) * h, time);
	return glm::normalize(n);
}

char shade(float d) {
	static constexpr std::string_view chars = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'.";
	return chars[chars.size() - static_cast<std::size_t>(d * chars.size())];
}

int main() {
	static constexpr auto screen_size = glm::i32vec2(64, 128);
	static constexpr auto camera_pos = glm::fvec3(0.f, 0.f, 1.f);
	static constexpr auto camera_scale = 0.4f;
	static constexpr auto light_pos = glm::fvec3(3.f, 4.f, 4.f);

	std::vector<char> out(screen_size.x * screen_size.y, ' ');

	auto time_begin = std::chrono::high_resolution_clock::now();
	while (true) {
		auto time_now = std::chrono::high_resolution_clock::now();
		using f_ms = std::chrono::duration<double, std::milli>;
		auto dt = std::chrono::duration_cast<f_ms>(time_now - time_begin).count() / 1000.;

		std::size_t k = 0;
		for (std::int32_t i = 0; i < screen_size.x; ++i) {
			for (std::int32_t j = 0; j < screen_size.y; ++j) {
				auto offset = glm::fvec2(i * 2 - screen_size.x, j * 2 - screen_size.y) / glm::fvec2(screen_size);
				auto pos = camera_pos + glm::fvec3(offset * camera_scale, 0.f);
				auto hit = march(pos, glm::fvec3(0.f, 0.f, -1.f), dt);
				if (!hit) {
					std::cout << ' ';
					out[k++] = ' ';
					continue;
				}

				static constexpr auto ambient = 0.1f;
				auto n = calcNormal(*hit, dt);
				auto l = glm::normalize(light_pos - *hit);
				auto d = glm::max(glm::dot(n, l), 0.f) + ambient;
				std::cout << shade(abs(d));
				out[k++] = shade(d);
			}
			std::cout << '\n';
			out[k++] = '\n';
		}
		clear();
		std::copy(out.begin(), out.end(), std::ostream_iterator<char>(std::cout));
	}
	return 0;
}
