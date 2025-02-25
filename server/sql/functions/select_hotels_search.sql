CREATE OR REPLACE FUNCTION select_hotels_search(
	_room_params boolean[],
    _num_for_sort text,
	_num_of_people int,
	_limit int,
    _offset int,
	_sort_cond text,
	_sort_type text,
	_country text,
	_city text
        ) 
        RETURNS TABLE(name character varying(60), price numeric(10,2), country character varying(30), city character varying(30), hotels_address character varying(60),user_rating numeric(2,1), stars int, double_bed int, single_bed int, free_cancel boolean ,photos_path text) AS $$
    BEGIN
         RETURN QUERY
        	SELECT hotels.name, room.price AS price,hotels.country, hotels.city, hotels.address,hotels.user_rating, hotels.stars, service_of_rooms.double_bed , service_of_rooms.single_bed, service_of_rooms.free_cancel ,hotels_photos.h_photos_path
		FROM hotels
		JOIN hotels_photos ON hotels_photos.fk_hotel_id = hotels.id
		JOIN service_of_rooms ON service_of_rooms.fk_hotel_id = hotels.id
		JOIN room ON room.fk_hotel_id = hotels.id AND room.id = service_of_rooms.fk_room_id
		WHERE room.price = (SELECT MIN(room.price) FROM room WHERE room.fk_hotel_id = hotels.id)
		AND(_num_for_sort IS NULL OR _sort_cond >= _num_for_sort)
			AND (_country IS NULL OR hotels.country = _country)
			AND (_city IS NULL OR hotels.city = _city) 
			AND (_room_params[1] IS NULL OR climate_control = _room_params[1]) --_room_params[0] - bool type of the filter 'climate control'
			AND (_room_params[2] IS NULL OR high_speed_wifi = _room_params[2]) --_room_params[1] - bool type of the filter 'high speed wifi'
			AND (_room_params[3] IS NULL OR p_bathroom = _room_params[3])  -- _room_params[2] - bool type of the filter 'private bathroom'
			AND (_room_params[4] IS NULL OR tv_add_opt = _room_params[4]) --_room_params[3] - bool type of the filter 'tv additional options'
			AND (_room_params[5] IS NULL OR mini_bar = _room_params[5])  -- _room_params[4] - bool type of the filter 'mini bar'
			AND (_room_params[6] IS NULL OR in_room_safe = _room_params[6]) -- _room_params[5] - bool type of the filter 'The availability of a safe in the room'
			AND (_room_params[7] IS NULL OR work_features = _room_params[7]) -- _room_params[6] - bool type of the filter 'features for work(accessories)'
			AND (_room_params[8] IS NULL OR room_service = _room_params[8]) -- _room_params[7] - bool type of the filter 'room service'
			AND (_room_params[9] IS NULL OR room_cleaning = _room_params[9]) -- _room_params[8] - bool type of the filter 'room cleaning'
			AND (_room_params[10] IS NULL OR transport_service = _room_params[10]) -- _room_params[9] - bool type of the filter 'transport service'
			AND (_room_params[11] IS NULL OR luggage_storage = _room_params[11]) -- _room_params[10] - bool type of the filter 'luggage storage'
			AND (_room_params[12] IS NULL OR full_time_supp = _room_params[12]) -- _room_params[11] - bool type of the filter 'full time support' 
			AND (_room_params[13] IS NULL OR fitness_center = _room_params[13]) -- _room_params[12] - bool type of the filter 'The availability of a fitness center'
			AND (_room_params[14] IS NULL OR in_room_workout_equip = _room_params[14]) -- _room_params[13] - bool type of the filter 'The availability of workout equipment in a room'
			AND (_room_params[15] IS NULL OR spa = _room_params[15]) -- _room_params[14] - bool type of the filter 'The availability of spa'
			AND (_room_params[16] IS NULL OR swimming_pool_cov = _room_params[16]) -- _room_params[15] - bool type of the filter 'The availability of a covered swimming pool'
			AND (_room_params[17] IS NULL OR swimming_pool_unc = _room_params[17]) -- _room_params[16] - bool type of the filter 'The availability of a uncovered swimming pool'
			AND (_room_params[18] IS NULL OR business_center = _room_params[18]) -- _room_params[17] - bool type of the filter 'The availability of a business center'
			AND (_room_params[19] IS NULL OR meeting_rooms = _room_params[19]) -- _room_params[18] - bool type of the filter 'The availability of a meeting room' 
			AND (_room_params[20] IS NULL OR restaurant = _room_params[20]) -- _room_params[19] - bool type of the filter 'The availability of a restaurant'
			AND (_room_params[21] IS NULL OR breakfast_opt = _room_params[21]) -- _room_params[20] - bool type of the filter 'breakfast option'
			AND (_room_params[22] IS NULL OR in_room_dining = _room_params[22])  -- _room_params[21] - bool type of the filter 'The availability of an option 'in room dinning''
			AND (_num_of_people IS NULL OR group_of >= _num_of_people) 
			AND (_room_params[23] IS NULL OR child_service = _room_params[23]) -- _room_params[22] - bool type of the filter 'The availability of services for kids' 
			AND (_room_params[24] is NULL OR pet_service = _room_params[24]) -- _room_params[23] - bool type of the filter 'The availability of services for pets' 
			AND (_room_params[25] is NULL OR service_of_rooms.free_cancel = _room_params[25]) -- _room_params[24] - bool type of the filter 'free cancel'
		GROUP BY hotels.name, room.price,hotels.country, hotels.city, hotels.address,hotels.user_rating, hotels.stars, service_of_rooms.double_bed , service_of_rooms.single_bed, service_of_rooms.free_cancel ,hotels_photos.h_photos_path
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
                WHEN 'stars' THEN hotels.stars::int
                WHEN '((stars + user_rating) / 2)' THEN ((hotels.stars + hotels.user_rating) / 2)
                ELSE hotels.user_rating
            END
    END ASC
        OFFSET _offset
		LIMIT _limit;

    END;
$$ LANGUAGE plpgsql;