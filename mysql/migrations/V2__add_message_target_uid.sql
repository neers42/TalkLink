-- messages に宛先ユーザー列を追加する。
--   target_uid = ''       : ルーム全員向け（ブロードキャスト。従来の挙動）
--   target_uid = <uid>    : その uid のみ向け（ADD_MEMBER 時の新規メンバーへの CREATE_ROOM 通知など）
ALTER TABLE messages
    ADD COLUMN target_uid VARCHAR(28) NOT NULL DEFAULT '';

CREATE INDEX idx_messages_target_uid ON messages (target_uid);
