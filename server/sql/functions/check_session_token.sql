CREATE OR REPLACE FUNCTION check_token_expiration(token_value text, time_now bigint) 
RETURNS boolean AS 
$$
BEGIN
    RETURN EXISTS (
        SELECT 1 
        FROM session
        WHERE token = token_value AND expired > time_now
    );
END;
$$ 
LANGUAGE plpgsql;