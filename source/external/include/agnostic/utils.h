#pragma once

// MACROS
#define DeleteNull(x) if(x != nullptr) { delete x; x = nullptr; }
#define DeleteArrayNull(x) if(x != nullptr) { delete[] x; x = nullptr; }
#define ReleaseNull(x) if(x != nullptr) { x->Release(); x = nullptr; }