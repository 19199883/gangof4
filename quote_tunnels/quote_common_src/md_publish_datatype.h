#if !defined(MD_DATATYPE_OF_PUBLISH_H_)
#define MD_DATATYPE_OF_PUBLISH_H_

#define DLL_PUBLIC  __attribute__ ((visibility("default")))

#pragma pack(push)
#pragma pack(8)

struct DLL_PUBLIC DataAnyType
{
	int type;
	int length;
	char buffer[1];
};

#pragma pack(pop)

#endif
