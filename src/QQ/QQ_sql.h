#include <string>
using std::string;

static string create_table_sql = R"(
CREATE TABLE friend_list(
    user_id INT UNSIGNED NOT NULL UNIQUE,
    user_nickname char(40),
    remark char(40)
);
CREATE TABLE ufriend_list(
    user_id INT UNSIGNED NOT NULL UNIQUE,
    user_nickname char(40),
    source char(100)
);
CREATE TABLE group_list(
    group_id INT UNSIGNED NOT NULL UNIQUE,
    group_name char(40),
    group_member_count INT UNSIGNED,
    group_max_member_count INT UNSIGNED,
    group_create_time INT UNSIGNED,
    group_level INT UNSIGNED,
    group_remark char(40)
);
CREATE TABLE group_member_list(
    user_id INT UNSIGNED NOT NULL,
    group_id INT UNSIGNED NOT NULL,
    user_nickname char(40),
    user_age INT UNSIGNED,
    user_area char(30),
    user_gender INT UNSIGNED,
    group_nickname char(40),
    group_nickname_changeable INT UNSIGNED,
    group_join_time INT UNSIGNED,
    group_last_active_time INT UNSIGNED,
    group_level char(10),
    group_role INT UNSIGNED,
    group_mute_time INT UNSIGNED,
    group_title char(30),
    group_title_expire_time INT UNSIGNED,
    group_is_unfriendly INT UNSIGNED
);
)";