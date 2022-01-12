#include "pch.h"

constexpr uintptr_t gNames_Offset = 0x01429664;
constexpr uintptr_t gObjects_Offset = 0x01417620;

struct UName {
    char unknown[8];
    uint64_t Index;
    char Name[1024];
};

struct FName
{
	int32_t Index;
	int32_t Number;
};

struct UObject
{
	uintptr_t		VTableObject;
	uint32_t		Index;
	char			UnknownData00[0x1C];
	UObject*        Outer;
	FName			Name;
	UObject*        Class;
};

template<typename T> class TArray {

public:

    constexpr int32_t size() {
        return number;
    }

    T* GetById(std::ptrdiff_t Index) {

        for (int32_t i = 0; i <= number; i++)
			if (gItems[i] != nullptr && gItems[i]->Index == Index)
                return gItems[i];

        return nullptr;
    }

private:

    T** gItems;
    int32_t number;
    int32_t max;
};

void DumpNames(TArray<UName>& gNames) {

	std::ofstream file("c:/SDK_GEN/NamesDump.txt");
	file << std::format("Address of gNames: 0x{:X}\n\n", gNames_Offset);

	for (int32_t i = 0; i <= gNames.size(); i++) {

		if (auto name = gNames.GetById(i)) {
			auto fName = std::format("Name[{:0>6}] {}", i >> 1, name->Name);
			file << fName << std::endl;
		}
	}
}

void DumpObjects(TArray<UObject>& gObjects, TArray<UName>& gNames) {

	std::ofstream file("c:/SDK_GEN/ObjectsDump.txt");
	file << std::format("Address of gObjects: 0x{:X}\n\n", gObjects_Offset);

	for (int32_t i = 0; i <= gObjects.size(); i++) {

		if (auto Object = gObjects.GetById(i)) {

			std::string name;
			std::string strType;

			if (Object->Class != nullptr)
				if(auto Type = gNames.GetById(Object->Class->Name.Index << 1))
					strType = std::format("<{}>", Type->Name);

			for (UObject* outerObj = Object->Outer; outerObj; outerObj = outerObj->Outer)
				if (auto outerName = gNames.GetById(outerObj->Name.Index << 1))
					name.insert(0, std::string(outerName->Name).append("."));

			if (auto baseName = gNames.GetById(Object->Name.Index << 1))
				name.append(baseName->Name);
			
			std::string fullName = strType.empty()
				? std::format("Object[{:0>6}] {:<133} 0x{:X}", i, name, (uintptr_t)Object)
				: std::format("Object[{:0>6}] {:^32} {:<100} 0x{:X}", i, strType, name, (uintptr_t)Object);

// 			if (strType.empty()) fullName = std::format("Object[{:0>6}] {:<133} 0x{:X}", i, name, (uintptr_t)Object);
// 			else fullName = std::format("Object[{:0>6}] {:^32} {:<100} 0x{:X}", i, strType, name, (uintptr_t)Object);
			
			file << fullName << std::endl;
		}
	}
}

int onAttach(HMODULE hModule) {

    auto gNames = *(TArray<UName>*)gNames_Offset;
    DumpNames(gNames);

    auto gObjects = *(TArray<UObject>*)gObjects_Offset;
    DumpObjects(gObjects, gNames);

    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  dwReason, LPVOID lpReserved) {

    if (dwReason == DLL_PROCESS_ATTACH) {

        DisableThreadLibraryCalls(hModule);

        HANDLE hThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)onAttach, hModule, 0, 0);
        if(hThread) CloseHandle(hThread);
    }

    return TRUE;
}

