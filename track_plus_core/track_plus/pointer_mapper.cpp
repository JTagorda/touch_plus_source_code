/*
 * Touch+ Software
 * Copyright (C) 2015
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Aladdin Free Public License as
 * published by the Aladdin Enterprises, either version 9 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Aladdin Free Public License for more details.
 *
 * You should have received a copy of the Aladdin Free Public License
 * along with this program.  If not, see <http://ghostscript.com/doc/8.54/Public.htm>.
 */

#include "pointer_mapper.h"
#include "console_log.h"
#include "pose_estimator.h"

void PointerMapper::compute(HandResolver& hand_resolver, Reprojector& reprojector)
{
	active = false;

	compute_cursor_point(index_down, hand_resolver.pt_precise_index0, hand_resolver.pt_precise_index1,
						 pt_index, reprojector, pt_cursor_index, dist_cursor_index_plane, actuate_dist, "compute_index");

	compute_cursor_point(thumb_down, hand_resolver.pt_precise_thumb0, hand_resolver.pt_precise_thumb1,
						 pt_thumb, reprojector, pt_cursor_thumb, dist_cursor_thumb_plane, actuate_dist + 10, "compute_thumb");

	if (pt_cursor_index.y > 1500)
	{
		PoseEstimator::pose_name = "";
		index_down = false;
	}

	if (calibrated)
	{
		if (!index_down)
			thumb_down = false;

		compute_pinch_to_zoom(hand_resolver);
	}
}

void PointerMapper::add_calibration_point(const uchar index)
{
	if (index == 0)
		pt_calib_vec0.push_back(pt_index);
	else if (index == 1)
		pt_calib_vec1.push_back(pt_index);
	else if (index == 2)
		pt_calib_vec2.push_back(pt_index);
	else if (index == 3)
		pt_calib_vec3.push_back(pt_index);
}

void PointerMapper::reset_calibration(const uchar index)
{
	if (index == 0)
		pt_calib_vec0.clear();
	else if (index == 1)
		pt_calib_vec1.clear();
	else if (index == 2)
		pt_calib_vec2.clear();
	else if (index == 3)
		pt_calib_vec3.clear();
}

void PointerMapper::compute_calibration_points()
{
	vector<float> x_vec;
	vector<float> y_vec;
	vector<float> z_vec;

	float x_median;
	float y_median;
	float z_median;

	if (pt_calib_vec0.size() > 0 && pt_calib_vec0.size() > 1 && pt_calib_vec0.size() > 2 && pt_calib_vec0.size() > 3)
	{
		x_vec = vector<float>();
		y_vec = vector<float>();
		z_vec = vector<float>();

		for (Point3f& pt : pt_calib_vec0)
		{
			x_vec.push_back(pt.x);
			y_vec.push_back(pt.y);
			z_vec.push_back(pt.z);
		}

		sort(x_vec.begin(), x_vec.end());
		sort(y_vec.begin(), y_vec.end());
		sort(z_vec.begin(), z_vec.end());

		x_median = x_vec[x_vec.size() * 0.5];
		y_median = y_vec[y_vec.size() * 0.5];
		z_median = z_vec[z_vec.size() * 0.5];

		pt_calib0 = Point3f(x_median, y_median, z_median);

		x_vec = vector<float>();
		y_vec = vector<float>();
		z_vec = vector<float>();

		for (Point3f& pt : pt_calib_vec1)
		{
			x_vec.push_back(pt.x);
			y_vec.push_back(pt.y);
			z_vec.push_back(pt.z);
		}

		sort(x_vec.begin(), x_vec.end());
		sort(y_vec.begin(), y_vec.end());
		sort(z_vec.begin(), z_vec.end());

		x_median = x_vec[x_vec.size() * 0.5];
		y_median = y_vec[y_vec.size() * 0.5];
		z_median = z_vec[z_vec.size() * 0.5];

		pt_calib1 = Point3f(x_median, y_median, z_median);

		x_vec = vector<float>();
		y_vec = vector<float>();
		z_vec = vector<float>();

		for (Point3f& pt : pt_calib_vec2)
		{
			x_vec.push_back(pt.x);
			y_vec.push_back(pt.y);
			z_vec.push_back(pt.z);
		}

		sort(x_vec.begin(), x_vec.end());
		sort(y_vec.begin(), y_vec.end());
		sort(z_vec.begin(), z_vec.end());

		x_median = x_vec[x_vec.size() * 0.5];
		y_median = y_vec[y_vec.size() * 0.5];
		z_median = z_vec[z_vec.size() * 0.5];

		pt_calib2 = Point3f(x_median, y_median, z_median);

		x_vec = vector<float>();
		y_vec = vector<float>();
		z_vec = vector<float>();

		for (Point3f& pt : pt_calib_vec3)
		{
			x_vec.push_back(pt.x);
			y_vec.push_back(pt.y);
			z_vec.push_back(pt.z);
		}

		sort(x_vec.begin(), x_vec.end());
		sort(y_vec.begin(), y_vec.end());
		sort(z_vec.begin(), z_vec.end());

		x_median = x_vec[x_vec.size() * 0.5];
		y_median = y_vec[y_vec.size() * 0.5];
		z_median = z_vec[z_vec.size() * 0.5];

		pt_calib3 = Point3f(x_median, y_median, z_median);

    	plane = Plane(pt_calib0, pt_calib1, pt_calib2);
	    direction_plane = plane.normal;

	    bool b0 = project_to_plane(pt_calib0, pt_calib0_projected, dist_calib0_plane);
		bool b1 = project_to_plane(pt_calib1, pt_calib1_projected, dist_calib1_plane);
		bool b2 = project_to_plane(pt_calib2, pt_calib2_projected, dist_calib2_plane);
		bool b3 = project_to_plane(pt_calib3, pt_calib3_projected, dist_calib3_plane);

		if (b0 && b1 && b2 && b3)
		{
			rect_warper.setSource(pt_calib0_projected.x, pt_calib0_projected.y, pt_calib1_projected.x, pt_calib1_projected.y,
								  pt_calib2_projected.x, pt_calib2_projected.y, pt_calib3_projected.x, pt_calib3_projected.y);

			rect_warper.setDestination(0, 0, 1000, 0, 1000, 1000, 0, 1000);

			calibrated = true;
		}
	}
	else
		console_log("not calibrated");
}

bool PointerMapper::project_to_plane(Point3f& pt, Point3f& result, float& dist_to_plane)
{
    Ray ray = Ray(pt, direction_plane);

    if (ray.intersects(plane, dist_to_plane))
    {
    	const float x = ray.position.x + dist_to_plane * ray.direction.x;
    	const float y = ray.position.y + dist_to_plane * ray.direction.y;
    	const float z = ray.position.z + dist_to_plane * ray.direction.z;
        result = Point3f(x, y, z);
        
        return true;
    }

    return false;
}

void PointerMapper::compute_cursor_point(bool& target_down, Point2f& pt_target0, Point2f& pt_target1, Point3f& pt_target,
										 Reprojector& reprojector, Point2f& pt_cursor, float& dist_cursor_target_plane,
										 const float actuation_dist, string name)
{
	LowPassFilter* low_pass_filter = value_store.get_low_pass_filter("low_pass_filter" + name);

	if (do_reset)
	{
		do_reset = false;
		low_pass_filter->reset();
	}

	if (pt_target0.x != -1 && pt_target1.x != -1)
	{
		active = true;
		pt_target = reprojector.reproject_to_3d(pt_target0.x, pt_target0.y, pt_target1.x, pt_target1.y);

		if (calibrated)
		{
			Point3f pt_target_projected;
			float dist_target_plane;
			bool b = project_to_plane(pt_target, pt_target_projected, dist_target_plane);

			if (b)
			{
				rect_warper.warp(pt_target_projected.x, pt_target_projected.y, pt_cursor.x, pt_cursor.y);

				if (value_store.has_point2f("pt_cursor" + name))
				{
	                Point2f temp = value_store.get_point2f("pt_cursor" + name);
	                float alpha = get_distance(pt_cursor, temp, true) / 1000;
		            if (alpha < 0.1)
		            	alpha = 0.1;
		            else if (alpha > 1)
		            	alpha = 1;

		            // low_pass_filter->compute(alpha, 0.5, "alpha" + name);
		            low_pass_filter->compute(pt_cursor, alpha, "pt_cursor" + name);
		        }
		        value_store.set_point2f("pt_cursor" + name, pt_cursor);

				if (dist_target_plane <= actuation_dist)
					value_store.set_bool("actuated" + name, true);

				if (value_store.get_bool("actuated" + name))
				{
					target_down = true;
					
					if (dist_target_plane > actuation_dist + 5)
					{
						target_down = false;
						value_store.set_bool("actuated" + name, false);
					}
				}

				dist_cursor_target_plane = dist_target_plane;
				low_pass_filter->compute(dist_cursor_target_plane, 0.5, "dist_cursor_target_plane" + name);
		    }
		}
	}
	else
		low_pass_filter->reset();
}

void PointerMapper::compute_pinch_to_zoom(HandResolver& hand_resolver)
{
	if (thumb_down && index_down)
		value_store.set_bool("thumb_actuated", true);
	if (/*dist_cursor_thumb_plane > 9999 || */!index_down)
		value_store.set_bool("thumb_actuated", false);

	bool has_thumb = (hand_resolver.pt_precise_thumb0.x != -1);

	const uchar has_thumb_vec_size_max = 3;
	int has_thumb_vec_index = value_store.get_int("has_thumb_vec_index");

	vector<int>* has_thumb_vec = value_store.get_int_vec("has_thumb_vec");
	if (has_thumb_vec->size() < has_thumb_vec_size_max)
	{
		has_thumb_vec->push_back(has_thumb);
		++has_thumb_vec_index;
	}
	else if (has_thumb_vec_index < has_thumb_vec_size_max)
	{
		(*has_thumb_vec)[has_thumb_vec_index] = has_thumb;
		++has_thumb_vec_index;
	}
	else
	{
		has_thumb_vec_index = 0;
		(*has_thumb_vec)[has_thumb_vec_index] = has_thumb;
		++has_thumb_vec_index;
	}

	value_store.set_int("has_thumb_vec_index", has_thumb_vec_index);

	bool has_thumb_old = true;
	for (int i = 0; i < has_thumb_vec->size(); ++i)
		if ((*has_thumb_vec)[i] == false)
		{
			has_thumb_old = false;
			break;
		}

	const bool zoom_out = has_thumb && has_thumb_old;

	if (value_store.get_bool("thumb_actuated") && !value_store.get_bool("thumb_actuated_old"))
	{
		Point2f pt_anchor = Point2f((pt_cursor_index.x + pt_cursor_thumb.x) / 2, (pt_cursor_index.y + pt_cursor_thumb.y) / 2);
		Point2f pt_diff = Point2f(pt_cursor_index.x - pt_anchor.x, pt_cursor_index.y - pt_anchor.y);

		value_store.set_point2f("pt_anchor", pt_anchor);
		value_store.set_point2f("pt_diff", pt_diff);
	}

	if (value_store.get_bool("thumb_actuated"))
	{
		Point2f pt_anchor = value_store.get_point2f("pt_anchor");
		Point2f pt_diff = value_store.get_point2f("pt_diff");

		const float diff_x = (pt_cursor_index.x - pt_anchor.x) / 2;
		const float diff_y = (pt_cursor_index.y - pt_anchor.y) / 2;

		pt_pinch_to_zoom_index = Point2f(pt_anchor.x + diff_x + pt_diff.x / 2, pt_anchor.y + diff_y + pt_diff.y / 2);
		pt_pinch_to_zoom_thumb = Point2f(pt_anchor.x - diff_x + pt_diff.x / 2, pt_anchor.y - diff_y + pt_diff.y / 2);

		pinch_to_zoom = true;
	}
	else if (pinch_to_zoom)
		pinch_to_zoom = false;

	value_store.set_bool("thumb_actuated_old", value_store.get_bool("thumb_actuated"));
}

void PointerMapper::reset()
{
	do_reset = true;
}