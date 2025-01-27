CREATE OR REPLACE FUNCTION select_places_search(
    _num_for_sort text,
	_limit int,
    _offset int,
	_sort_cond text,
	_sort_type text,
    _country text,
    _city text
        ) 
        RETURNS TABLE(name character varying(60), country character varying(30), city character varying(30), description text, user_rating numeric(2,1), price numeric(10,2), is_open time without time zone, is_close time without time zone ,photos_path text) AS $$
    BEGIN
         RETURN QUERY
        	SELECT places.name,places.country, places.city, places.description,places.user_rating, places.price, places.is_open, places.is_close, places_photos.p_photos_path
		FROM places
		JOIN places_photos ON places_photos.fk_place_id = places.id
		WHERE (_num_for_sort IS NULL OR _sort_cond >= _num_for_sort)
            AND(_country IS NULL OR places.country = _country)
            AND(_city IS NULL OR places.city = _city)
        GROUP BY places.name,places.country, places.city, places.description,places.user_rating, places.price, places.is_open, places.is_close, places_photos.p_photos_path
		ORDER BY
    CASE 
        WHEN _sort_type = 'DESC' THEN
            CASE _sort_cond
                WHEN 'user_rating' THEN places.user_rating::numeric(2,1)
                WHEN 'price' THEN MIN(places.price)::numeric(10,2)
                ELSE places.user_rating
            END
    END DESC,
    CASE 
        WHEN _sort_type = 'ASC' THEN
            CASE _sort_cond
                WHEN 'user_rating' THEN places.user_rating::numeric(2,1)
                WHEN 'price' THEN MIN(places.price)::numeric(10,2)
                ELSE places.user_rating
            END
    END ASC
        OFFSET _offset
		LIMIT _limit;

    END;
$$ LANGUAGE plpgsql;