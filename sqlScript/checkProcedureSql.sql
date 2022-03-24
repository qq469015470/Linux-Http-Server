select concat('drop procedure ', b.SPECIFIC_NAME) from (SELECT * FROM information_schema.Routines WHERE ROUTINE_SCHEMA = 'dangkou') b;
