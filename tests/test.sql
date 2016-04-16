INSERT INTO configs (id, name, description) VALUES (1, 'Default', '');

INSERT INTO "values" (id_configs, id, id_parent, key, value, description) VALUES (1, 1, 0, 'folder', '', '');
INSERT INTO "values" (id_configs, id, id_parent, key, value, description) VALUES (1, 2, 1, 'key1', 'value key 1', '');
INSERT INTO "values" (id_configs, id, id_parent, key, value, description) VALUES (1, 3, 2, 'key1_2', 'value key 1 2', '');
INSERT INTO "values" (id_configs, id, id_parent, key, value, description) VALUES (1, 4, 1, 'key2', 'value key 2', '');
INSERT INTO "values" (id_configs, id, id_parent, key, value, description) VALUES (1, 6, 2, 'key1_1', 'value key 1 1', '');
INSERT INTO "values" (id_configs, id, id_parent, key, value, description) VALUES (1, 5, 4, 'key2-1', 'value key 2 1', '');
