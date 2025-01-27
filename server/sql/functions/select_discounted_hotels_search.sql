CREATE OR REPLACE FUNCTION select_discounted_hotels_start(
    _num_for_sort text,
	_num_of_people int,
	_limit int,
    _offset int,
	_sort_cond text,
	_sort_type text,
	_climate_control boolean,
    _high_speed_wifi boolean,
    _private_bathroom boolean,
    _tv_add_opt boolean,
    _mini_bar boolean,
    _in_room_safe boolean,
    _work_features boolean,
    _room_service boolean,
    _room_cleaning boolean,
    _transport_service boolean,
    _luggage_storage boolean,
    _full_time_support boolean,
    _fitness_center boolean,
    _in_room_workout_equip boolean,
    _spa boolean,
    _swimming_pool_cov boolean,
    _swimming_pool_unc boolean,
    _business_center boolean,
    _meeting_rooms boolean,
    _restaurant boolean,
    _breakfast_options boolean,
    _in_room_dining boolean,
    _children_service boolean,
    _pet boolean,
    _free_cancel boolean
        ) 
        RETURNS TABLE(name character varying(60), country character varying(30), city character varying(30), user_rating numeric(2,1), old_price numeric(10,2), new_price numeric(10,2) ,photos_path text) AS $$
    BEGIN
         RETURN QUERY
        	SELECT hotels.name , hotels.country, hotels.city, hotels.user_rating, room.price, (room.price * discounted_hotels.discount) AS new_price ,h_photos_path
		FROM hotels
		JOIN hotels_photos ON hotels_photos.fk_hotel_id = hotels.id
		JOIN service_of_rooms ON service_of_rooms.fk_hotel_id = hotels.id
        JOIN discounted_hotels ON discounted_hotels.fk_hotel_id = hotels.id
		JOIN room ON room.fk_hotel_id = hotels.id
		WHERE (_sort_cond >= _num_for_sort) 
            AND (_climate_control IS NULL OR climate_control = _climate_control) 
			AND (_high_speed_wifi IS NULL OR high_speed_wifi = _high_speed_wifi) 
			AND (_private_bathroom IS NULL OR p_bathroom = _private_bathroom) 
			AND (_tv_add_opt IS NULL OR tv_add_opt = _tv_add_opt) 
			AND (_mini_bar IS NULL OR mini_bar = _mini_bar) 
			AND (_in_room_safe IS NULL OR in_room_safe = _in_room_safe) 
			AND (_work_features IS NULL OR work_features = _work_features) 
			AND (_room_service IS NULL OR room_service = _room_service) 
			AND (_room_cleaning IS NULL OR room_cleaning = _room_cleaning) 
			AND (_transport_service IS NULL OR _transport_service = _transport_service) 
			AND (_luggage_storage IS NULL OR luggage_storage = _luggage_storage) 
			AND (_full_time_support IS NULL OR full_time_supp = _full_time_support) 
			AND (_fitness_center IS NULL OR fitness_center = _fitness_center) 
			AND (_in_room_workout_equip IS NULL OR in_room_workout_equip = _in_room_workout_equip) 
			AND (_spa IS NULL OR spa = _spa) 
			AND (_swimming_pool_cov IS NULL OR swimming_pool_cov = _swimming_pool_cov) 
			AND (_swimming_pool_unc IS NULL OR swimming_pool_unc = _swimming_pool_unc) 
			AND (_business_center IS NULL OR business_center = _business_center) 
			AND (_meeting_rooms IS NULL OR meeting_rooms = _meeting_rooms) 
			AND (_restaurant IS NULL OR restaurant = _restaurant) 
			AND (_breakfast_options IS NULL OR breakfast_opt = _breakfast_options) 
			AND (_in_room_dining IS NULL OR in_room_dining = _in_room_dining) 
			AND (_num_of_people IS NULL OR group_of >= _num_of_people) 
			AND (_children_service IS NULL OR child_service = _children_service) 
			AND (_pet is NULL OR pet_service = _pet) 
			AND (_free_cancel is NULL OR free_cancel = _free_cancel)
		GROUP BY hotels.name, hotels.country, hotels.city, h_photos_path, hotels.user_rating, hotels.stars
		ORDER BY 
    CASE 
        WHEN _sort_type = 'DESC' THEN
            CASE _sort_cond
                WHEN 'user_rating' THEN hotels.user_rating::numeric(2,1)
                WHEN 'price' THEN MIN(room.price)::numeric(10,2)
                WHEN 'stars' THEN hotels.stars::int
                WHEN '((stars + user_rating) / 2)' THEN ((hotels.stars + hotels.user_rating) / 2)
                ELSE hotels.user_rating
            END
    END DESC,
    CASE 
        WHEN _sort_type = 'ASC' THEN
            CASE _sort_cond
                WHEN 'user_rating' THEN hotels.user_rating::numeric(2,1)
                WHEN 'price' THEN MIN(room.price)::numeric(10,2)
                WHEN 'stars' THEN stars::int
                WHEN '((stars + user_rating) / 2)' THEN ((hotels.stars + hotels.user_rating) / 2)
                ELSE hotels.user_rating
            END
    END ASC
        OFFSET _offset
		LIMIT _limit;

    END;
$$ LANGUAGE plpgsql;