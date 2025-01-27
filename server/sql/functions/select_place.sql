CREATE OR REPLACE FUNCTION select_place(
	_id integer
        ) 
        RETURNS TABLE(name character varying(60), city character varying(30), address character varying(60), user_rating numeric(2,1), price numeric(10,2), description text, photos_path text, hotel text[]) AS $$
    BEGIN
        RETURN QUERY
        	SELECT places.name, places.city, places.address, places.user_rating, places.price,places.description, places_photos.p_photos_path, array_agg(
                DISTINCT (
                    hotels.name || ', ' ||
                    hotels.city || ', ' ||
                    hotels.country || ', ' ||
                    hotels_photos.h_photos_path
                )
            ) AS hotels
	        FROM places
	        JOIN hotels ON hotels.country = places.country AND hotels.city = places.city
	        JOIN places_photos ON places_photos.fk_place_id = places.id
	        JOIN hotels_photos ON hotels_photos.fk_hotel_id = hotels.id 
            WHERE _id = places.id
	        GROUP BY places.name, places.city, places.address, places.user_rating, places.price,places.description, places_photos.p_photos_path;
    END;
$$ LANGUAGE plpgsql;