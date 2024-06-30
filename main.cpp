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

auto donut_sdf(glm::dvec3 p, double time) {
	auto r = glm::rotateX(p, glm::radians(30. * time));
	r = glm::rotateZ(r, glm::radians(70. * time));

	static constexpr auto t = glm::dvec2(0.25, 0.05);
	auto xz = glm::dvec2(r.x, r.z);
	auto q = glm::dvec2(length(xz) - t.x, r.y);
	return length(q) - t.y;
}

std::optional<glm::dvec3> march(glm::dvec3 initial_pos, glm::dvec3 dir, double time) {
	dir = glm::normalize(dir);
	auto pos = initial_pos;
	double dist;
	while (!glm::epsilonEqual(0., (dist = donut_sdf(pos, time)), std::numeric_limits<double>::epsilon())) {
		if (dist < 1E-5)
			break;

		pos += dir * dist;

		if (glm::distance(initial_pos, pos) >= 10.)
			return std::nullopt;
	}
	return pos;
}

// See https://iquilezles.org/articles/normalsSDF/
glm::dvec3 calcNormal(glm::dvec3 p, double time) {
	const auto h = 0.0001;
	const auto k = glm::dvec2(1, -1);
	auto n =
			glm::xyy(k) * donut_sdf(p + glm::xyy(k) * h, time) +
			glm::yyx(k) * donut_sdf(p + glm::yyx(k) * h, time) +
			glm::yxy(k) * donut_sdf(p + glm::yxy(k) * h, time) +
			glm::xxx(k) * donut_sdf(p + glm::xxx(k) * h, time);
	return glm::normalize(n);
}

char shade(double d) {
	static constexpr std::string_view chars = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'.";
	return chars[chars.size() - std::llround(d * (chars.size() - 1))];
}

int main() {
	static constexpr auto screen_size = glm::i32vec2(64, 128);
	static constexpr auto camera_pos = glm::dvec3(0., 0., 1.);
	static constexpr auto camera_scale = 0.4;
	static constexpr auto light_pos = glm::dvec3(3., 4., 4.);

	std::vector<char> out(screen_size.x * screen_size.y, ' ');

	auto time_begin = std::chrono::high_resolution_clock::now();
	while (true) {
		auto time_now = std::chrono::high_resolution_clock::now();
		using f_ms = std::chrono::duration<double, std::milli>;
		auto dt = std::chrono::duration_cast<f_ms>(time_now - time_begin).count() / 1000.;

		std::size_t k = 0;
		for (std::int32_t i = 0; i < screen_size.x; ++i) {
			for (std::int32_t j = 0; j < screen_size.y; ++j) {
				auto offset = glm::dvec2(i * 2 - screen_size.x, j * 2 - screen_size.y) / glm::dvec2(screen_size);
				auto pos = camera_pos + glm::dvec3(offset * camera_scale, 0.f);
				auto hit = march(pos, glm::dvec3(0.f, 0.f, -1.f), dt);
				if (!hit) {
					out[k++] = ' ';
					continue;
				}

				static constexpr auto ambient = 0.1;
				auto n = calcNormal(*hit, dt);
				auto l = glm::normalize(light_pos - *hit);
				auto d = glm::max(glm::dot(n, l), 0.) + ambient;
				out[k++] = shade(d);
			}
			out[k++] = '\n';
		}
		clear();
		std::copy(out.begin(), out.end(), std::ostream_iterator<char>(std::cout));
	}
	return 0;
}
