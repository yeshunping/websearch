struct StructureKV{
	1: i32 key;
	2: string value;
}

struct StructureData{
	1: i32 version;
	2: i32 type;
	3: list <StructureKV> all;
}

