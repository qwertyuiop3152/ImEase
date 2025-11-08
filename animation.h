#ifndef REN_ANIMATION_H
#define REN_ANIMATION_H
#include <imgui.h>
#include <memory>
#include <string>
#include <cmath>
#include <functional>
#include <algorithm>

typedef ImVec2 vec2;
typedef ImVec4 vec4;

namespace evo::ren
{
	inline constexpr auto PI = 3.14159265358979323846f;
	inline constexpr auto PI2 = 1.57079632679489661923f;

	enum : unsigned char
	{
		i_linear = 0,
		i_ease_in,
		i_ease_out,
		i_ease_in_out,
		i_elastic_in,
		i_elastic_out,
		i_elastic_in_out,
		i_bounce_in,
		i_bounce_out,
		i_bounce_in_out,
		i_max
	};

	inline float lerp(float a, float b, float u) {
		return a + (b - a) * u;
	}

	using Fn = float(*)(float a, float b, float u);

	inline float ease_linear(float a, float b, float u) {
		return lerp(a, b, std::clamp(u, 0.f, 1.f));
	}

	inline float ease_in(float a, float b, float u) {
		u = std::clamp(u, 0.f, 1.f);
		return lerp(a, b, u * u * u);
	}

	inline float ease_out(float a, float b, float u) {
		u = std::clamp(u, 0.f, 1.f);
		float t = 1.f - u;
		return lerp(a, b, 1.f - t * t * t);
	}

	inline float ease_in_out(float a, float b, float u) {
		u = std::clamp(u, 0.f, 1.f);
		float t = (u < .5f) ? (4.f * u * u * u) : (1.f - std::pow(-2.f * u + 2.f, 3.f) * .5f);
		return lerp(a, b, t);
	}

	inline float elastic_in(float a, float b, float u)
	{
		u = std::clamp(u, 0.f, 1.f);
		float t = u * u;
		return lerp(a, b, t * t * std::sinf(u * PI * 4.f));
	}

	inline float elastic_out(float a, float b, float u)
	{
		u = std::clamp(u, 0.f, 1.f);
		float t = (u - 1.f) * (u - 1.f);
		return lerp(a, b, 1.f - t * t * std::cosf(u * PI * 4.5f));
	}

	inline float elastic_in_out(float a, float b, float u) {
		u = std::clamp(u, 0.f, 1.f);
		if (u < .45f) {
			float t = u * u;
			return lerp(a, b, 8.f * t * t * std::sinf(u * PI * 9.f));
		}
		else if (u < .55f) {
			return lerp(a, b, .5f + .75f * std::sinf(u * PI * 4.f));
		}
		else {
			float t = (u - 1.f) * (u - 1.f);
			return lerp(a, b, 1.f - 8.f * t * t * std::sinf(u * PI * 9.f));
		}
	}

	inline float bounce_in(float a, float b, float u) {
		u = std::clamp(u, 0.f, 1.f);
		return lerp(a, b, std::pow(2.f, 6.f * (u - 1.f)) * std::fabs(std::sinf(u * PI * 3.5f)));
	}

	inline float bounce_out(float a, float b, float u) {
		u = std::clamp(u, 0.f, 1.f);
		return lerp(a, b, 1.f - std::pow(2.f, -6.f * u) * std::fabs(std::cosf(u * PI * 3.5f)));
	}
	inline float bounce_in_out(float a, float b, float u)
	{
		u = std::clamp(u, 0.f, 1.f);
		if (u < .5f)
			return lerp(a, b, 8.f * std::pow(2.f, 8.f * (u - 1.f)) * std::fabs(std::sinf(u * PI * 7.f)));
		return lerp(a, b, 1.f - 8.f * std::pow(2.f, -8.f * u) * std::fabs(std::sinf(u * PI * 7.f)));
	}

	/*
		index_ = i
	*/

	inline constexpr Fn Fns[i_max] = {
		&ease_linear,
		&ease_in,
		&ease_out,
		&ease_in_out,
		&elastic_in,
		&elastic_out,
		&elastic_in_out,
		&bounce_in,
		&bounce_out,
		&bounce_in_out		
	};

	// animation
	class anim_base : public std::enable_shared_from_this<anim_base>
	{
	public:
		virtual ~anim_base() = default;
		virtual void animate(float dt) {}
		template<typename T>
		std::shared_ptr<T> as() { return std::static_pointer_cast<T>(shared_from_this()); }
	};

	template<typename T>
	class anim : public anim_base
	{
	public:
		anim(const T& v, float d, unsigned char easing = i_linear) : value(v), end(v), start(v), duration(d), interpolation(easing)
		{

		}

		void animate(float dt) override {
			if (on_started && time == 0.f) on_started(as<anim<T>>());
			if (duration > 0.f && dt > 0.f) {
				time += (dt / duration);
				if (time > 1.f) time = 1.f;
			}
			else {
				time = 1.f;
			}

			if (on_finished) {
				if (time >= 1.f) {
					if (!did_call_finish) on_finished(as<anim<T>>());
					did_call_finish = true;
				}
				else
				{
					did_call_finish = false;
				}
			}
		}

		virtual void direct(const T& t) {
			start = value;
			end = t;
			time = 0.f;
		}

		virtual void direct(const T& a, const T& t) {
			start = a;
			end = t;
			time = 0.f;
		}

		T value{};
		T end{};
		T start{};

		std::function<void(std::shared_ptr<anim<T>>)> on_started{};
		std::function<void(std::shared_ptr<anim<T>>)> on_finished{};

		unsigned char interpolation{ i_linear };
		float duration{ 0.f };
	protected:
		float eval_scalar(float a, float b) const {
			return Fns[std::min<unsigned char>(interpolation, i_max - 1)](a, b, time);
		}

		float time{ 0.f };
		bool did_call_finish{ false };
	};

	class anim_float : public anim<float>
	{
	public:
		anim_float(float v, float d, unsigned char i = i_linear) : anim<float>(v, d, i)
		{

		}

		void animate(float dt) override {
			anim::animate(dt);
			value = eval_scalar(start, end);
		}

	};

	class anim_vec2 : public anim<vec2>
	{
	public:
		anim_vec2(vec2 v, float d, unsigned char i = i_linear) : anim<vec2>(v, d, i)
		{

		}

		void animate(float dt) override {
			anim::animate(dt);
			value.x = eval_scalar(start.x, end.x);
			value.y = eval_scalar(start.y, end.y);
		}

	};

	enum anim_color_space : unsigned char { ac_rgba, ac_hsva };

	class anim_color : public anim<vec4>
	{
	public:
		anim_color(vec4 v, float d, unsigned char i = i_linear, anim_color_space sp = ac_rgba)
			: anim<vec4>(v, d, i), space(sp) {}

		void animate(float dt) override {
			anim::animate(dt);
			if (space == ac_rgba)
			{
				value.x = eval_scalar(start.x, end.x);
				value.y = eval_scalar(start.y, end.y);
				value.z = eval_scalar(start.z, end.z);
				value.w = eval_scalar(start.w, end.w);
			}
			else {
				float sh, ss, sv; 
				ImGui::ColorConvertRGBtoHSV(start.x, start.y, start.z, sh, ss, sv);
				float eh, es, ev;
				ImGui::ColorConvertRGBtoHSV(end.x, end.y, end.z, eh, es, ev);

				auto lerp_hue = [&](float h0, float h1, float u)->float
					{
						float d = h1 - h0;
						if (d > .5f) h0 += 1.f;
						else if (d < -.5f) h1 += 1.f;
						float h = lerp(h0, h1, std::clamp(u, 0.f, 1.f));
						h = std::fmod(h, 1.f); if (h < 0.f) h += 1.f;
						return h;
					};
				float u = std::clamp(this->time, 0.f, 1.f);
				float ih = lerp_hue(sh, eh, u);
				float is = Fns[interpolation](ss, es, u);
				float iv = Fns[interpolation](sv, ev, u);
				ImGui::ColorConvertHSVtoRGB(ih, is, iv, value.x, value.y, value.z);
				value.w = Fns[interpolation](start.w, end.w, u);
			}
		}


		anim_color_space space{ ac_rgba };

	};


	inline float value_duration(const char* id, float target, float duration, unsigned char easing = i_ease_in_out)
	{
		struct slot
		{
			std::shared_ptr<anim_float> a;
			float last_target = FLT_MAX;
			float last_duration = -1.f;
			unsigned char last_easing = 255;
		};

		static std::unordered_map < std::string, slot> s;
		slot& sl = s[id];

		if (!sl.a) sl.a = std::make_shared<anim_float>(target, duration, easing);

		if (target != sl.last_target || duration != sl.last_duration || easing != sl.last_easing)
		{
			sl.a->duration = duration;
			sl.a->interpolation = easing;
			sl.a->direct(sl.a->value, target);
			sl.last_target = target;
			sl.last_duration = duration;
			sl.last_easing = easing;
		}
		sl.a->animate(ImGui::GetIO().DeltaTime);
		return sl.a->value;
	}


	inline vec2 vec2_duration(const char* id, vec2 target, float duration, unsigned char easing = i_ease_out)
	{
		struct slot
		{
			std::shared_ptr<anim_vec2> a;
			vec2 last_target = vec2(FLT_MAX, FLT_MAX);
			float last_duration = -1.f;
			unsigned char last_easing = 255;
		};

		static std::unordered_map<std::string, slot> s;

		slot& sl = s[id];
		if (!sl.a) sl.a = std::make_shared<anim_vec2>(target, duration, easing);

		if (target.x != sl.last_target.x || target.y != sl.last_target.y || duration != sl.last_duration || easing != sl.last_easing)
		{
			sl.a->duration = duration;
			sl.a->interpolation = easing;
			sl.a->direct(sl.a->value, target);
			sl.last_target = target;
			sl.last_duration = duration;
			sl.last_easing = easing;
		}

		sl.a->animate(ImGui::GetIO().DeltaTime);
		return sl.a->value;
	}


	inline void reset(const char* id = nullptr)
	{
		static std::unordered_map<std::string, int> dummy;
		(void)dummy;
	}


}


#endif