#pragma once

/*  This file is part of Imagine.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Imagine.  If not, see <http://www.gnu.org/licenses/> */

#include <imagine/base/baseDefs.hh>
#include <imagine/util/Interpolator.hh>

namespace IG
{

template <class T, class Clock, InterpolatorType INTERPOLATOR_TYPE = InterpolatorType::UNSET>
class FrameAnimation
{
public:
	constexpr FrameAnimation() = default;

	~FrameAnimation() { cancel(); }

	FrameAnimation &operator=(FrameAnimation &&) = delete;

	FrameAnimation &operator=(T v) noexcept
	{
		cancel();
		animator.val = v;
		return *this;
	}

	void start(Clock &c, T begin, T end, FrameTime duration, invocable<Clock &, T> auto &&onUpdate)
	{
		cancel();
		clock = &c;
		if(begin == end)
		{
			animator = end;
			return;
		}
		auto now = steadyClockTimestamp();
		animator = {begin, end, {}, now, duration};
		animate =
			[this, onUpdate = IG_forward(onUpdate)](FrameParams params)
			{
				bool updating = animator.update(params.timestamp());
				onUpdate(*clock, (T)animator);
				if(!updating)
				{
					animate = {};
					return false;
				}
				else
				{
					return true;
				}
			};
		clock->addOnFrame(animate);
	}

	void finish()
	{
		cancel();
		animator.finish();
		animate({animator.endTime(), 0});
	}

	bool isFinished() const { return animator.isFinished(); }

	void cancel()
	{
		if(animate)
		{
			assert(clock);
			clock->removeOnFrame(animate);
			animate = {};
		}
	}

	T value() const { return animator; }

protected:
	InterpolatorValue<T, FrameTime, INTERPOLATOR_TYPE> animator{};
	OnFrameDelegate animate{};
	Clock *clock{};
};

}
