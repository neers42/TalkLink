CREATE TABLE IF NOT EXISTS users (
    uid VARCHAR(28) NOT NULL PRIMARY KEY,
    name VARCHAR(28) NOT NULL,
    mail_address VARCHAR(256) NOT NULL,
    icon_url VARCHAR(255),
    created_at DATETIME NOT NULL,
    last_login DATETIME
);

CREATE TABLE IF NOT EXISTS chatrooms (
    room_id BINARY(16) NOT NULL PRIMARY KEY,
    room_name VARCHAR(50) NOT NULL,
    room_icon_url VARCHAR(255),
    created_at DATETIME
);

CREATE TABLE IF NOT EXISTS chat_members (
    room_id BINARY(16) NOT NULL,
    uid VARCHAR(28) NOT NULL,
    joined_at DATETIME,
    PRIMARY KEY(room_id, uid),
    FOREIGN KEY(room_id) REFERENCES chatrooms(room_id) ON DELETE CASCADE,
    FOREIGN KEY(uid) REFERENCES users(uid) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS messages (
    mid BIGINT PRIMARY KEY,
    room_id BINARY(16) NOT NULL,
    sender_uid VARCHAR(28) NOT NULL,
    send_time DATETIME NOT NULL,
    message JSON NOT NULL,
    room_sequence BIGINT UNSIGNED NOT NULL DEFAULT 0,
    FOREIGN KEY(room_id) REFERENCES chatrooms(room_id) ON DELETE CASCADE,
    FOREIGN KEY(sender_uid) REFERENCES users(uid) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS unsend_messages (
    mid BIGINT NOT NULL,
    uid VARCHAR(28) NOT NULL,
    room_id BINARY(16) NOT NULL,
    PRIMARY KEY(mid, uid, room_id),
    FOREIGN KEY(mid) REFERENCES messages(mid) ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY(uid) REFERENCES users(uid) ON DELETE CASCADE,
    FOREIGN KEY(room_id) REFERENCES chatrooms(room_id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS unsend_control_messages (
    mid BIGINT NOT NULL,
    uid VARCHAR(28) NOT NULL,
    room_id BINARY(16) NOT NULL,
    json_payload JSON NOT NULL,
    request_time DATETIME NOT NULL,
    PRIMARY KEY(mid, uid, room_id),
    FOREIGN KEY(mid) REFERENCES messages(mid) ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY(uid) REFERENCES users(uid) ON DELETE CASCADE,
    FOREIGN KEY(room_id) REFERENCES chatrooms(room_id) ON DELETE CASCADE
);