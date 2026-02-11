CREATE TABLE IF NOT EXISTS clients (
    cid VARCHAR(28) NOT NULL PRIMARY KEY,
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
    cid VARCHAR(28) NOT NULL,
    joined_at DATETIME,
    PRIMARY KEY(room_id, cid),
    FOREIGN KEY(room_id) REFERENCES chatrooms(room_id) ON DELETE CASCADE,
    FOREIGN KEY(cid) REFERENCES clients(cid) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS messages (
    mid BIGINT PRIMARY KEY,
    room_id BINARY(16) NOT NULL,
    sender_cid VARCHAR(28) NOT NULL,
    send_time DATETIME NOT NULL,
    message JSON NOT NULL,
    FOREIGN KEY(room_id) REFERENCES chatrooms(room_id) ON DELETE CASCADE,
    FOREIGN KEY(sender_cid) REFERENCES clients(cid) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS unsend_messages (
    mid BIGINT NOT NULL,
    cid VARCHAR(28) NOT NULL,
    room_id BINARY(16) NOT NULL,
    PRIMARY KEY(mid, cid, room_id),
    FOREIGN KEY(mid) REFERENCES messages(mid) ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY(cid) REFERENCES clients(cid) ON DELETE CASCADE,
    FOREIGN KEY(room_id) REFERENCES chatrooms(room_id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS unsend_control_messages (
    mid BIGINT NOT NULL,
    cid VARCHAR(28) NOT NULL,
    room_id BINARY(16) NOT NULL,
    json_payload JSON NOT NULL,
    request_time DATETIME NOT NULL,
    PRIMARY KEY(mid, cid, room_id),
    FOREIGN KEY(mid) REFERENCES messages(mid) ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY(cid) REFERENCES clients(cid) ON DELETE CASCADE,
    FOREIGN KEY(room_id) REFERENCES chatrooms(room_id) ON DELETE CASCADE
);