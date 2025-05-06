#include <shared/DirectXHelpers.hpp>
#include "nvse/PluginAPI.h"

// Hook related globals.
static CallDetour			kSLSUpdateLights;
static CallDetour			kSLSUpdateToggles;
static VirtFuncDetour		kShaderLightPropAddPasses[7];
static VirtFuncDetour		kSLSLoadStagesAndPasses;
static VirtFuncDetour		kSLSReinitialize;
static VirtFuncDetour		kSLSInitShaderConstants;
static uint32_t				uiAABBCheckBoundAddress = 0;
static uint32_t				uiNiAVObjectGetWorldBoundAddress = 0;
static uint32_t				uiShadowSceneNodeArrayAddress = 0;
static uint32_t				uiConstantMapAddEntryAddress = 0;
static uint32_t				uiConstantMapGetEntryAddress = 0;
static uint32_t				uiRenderPassArrayAddPassAddress = 0;
static uint32_t				uiShaderLightPropResortLights = 0;
static uint32_t				uiShaderLightPropGetFirstLightAddress = 0;
static uint32_t				uiShaderLightPropGetNextLightAddress = 0;
static uint32_t				uiRenderPassSetLightsAddress = 0;
static uint32_t				uiInLODWorldAddress = 0;
static uint32_t				uiCameraPosAddress = 0;
static uint32_t				uiHDRAddress = 0;
static uint32_t				uiInInteriorAddress = 0;
static uint32_t				uiSunlightDimmerAddress = 0;
static uint32_t				uiMagicNightEyeAmbientAddress = 0;
static uint32_t				uiSLSPixelConstantMapAddress = 0;
static uint32_t				uiSLSVertexConstantMapAddress = 0;
static uint32_t				uiSLSVertexConstantFlagsAddress = 0;
static uint32_t				uiSLSFogParamAddress = 0;
static uint32_t				uiSLSAmbientColorAddress = 0;
static uint32_t				uiSLSLightColorsAddress = 0;
static uint32_t				uiSLSLightPositionsAddress = 0;

// Constants.
static constexpr uint8_t	ucMaxPointlights = 24;
static constexpr float		fLightComponentMinValue = 1.0f / 255.0f;
static constexpr float		fLODLandShine = 30.0f;
static constexpr uint32_t	uiShaderLoaderVersion = 131;

// Game data and hooks.
class NiPoint3 {
public:
	float x;
	float y;
	float z;
};

class NiPoint4 {
public:
	float x;
	float y;
	float z;
	float w;
};

class NiColor {
public:
	float r;
	float g;
	float b;

	NiColor(float r, float g, float b) : r(r), g(g), b(b) {};
	NiColor(float f) : r(f), g(f), b(f) {};

	NiColor operator*=(const float f) {
		r *= f;
		g *= f;
		b *= f;
		return *this;
	};

	NiColor operator=(const NiPoint3& color) {
		r = color.x;
		g = color.y;
		b = color.z;
		return *this;
	};

	NiColor operator+(const NiColor& color) const {
		return NiColor(r + color.r, g + color.g, b + color.b);
	}

	NiColor operator*(const float f) {
		return NiColor(r * f, g * f, b * f);
	}
};

class NiColorA {
public:
	float r;
	float g;
	float b;
	float a;

	NiColorA(const NiColor& color) : r(color.r), g(color.g), b(color.b), a(1.0f) {};
	NiColorA(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) {};
	NiColorA(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {};

	NiColorA operator=(const NiPoint3& color) {
		r = color.x;
		g = color.y;
		b = color.z;
		return *this;
	};
};

template <class T_Data>
class NiPointer {
public:
	__forceinline NiPointer() : m_pObject(nullptr) {};
	__forceinline NiPointer(T_Data* apObject) : m_pObject(apObject) { if (m_pObject) m_pObject->IncRefCount(); }
	__forceinline NiPointer(const NiPointer& arPtr) : m_pObject(arPtr.m_pObject) { if (m_pObject) m_pObject->IncRefCount(); }
	__forceinline ~NiPointer() { if (m_pObject) m_pObject->DecRefCount(); }

	T_Data* m_pObject;

	__forceinline operator T_Data* () const { return m_pObject; }
	__forceinline T_Data& operator*() const { return *m_pObject; }
	__forceinline T_Data* operator->() const { return m_pObject; }

	__forceinline NiPointer<T_Data>& operator =(const NiPointer& ptr) {
		if (m_pObject != ptr.m_pObject) {
			if (m_pObject)
				m_pObject->DecRefCount();
			m_pObject = ptr.m_pObject;
			if (m_pObject)
				m_pObject->IncRefCount();
		}
		return *this;
	}

	__forceinline NiPointer<T_Data>& operator =(T_Data* pObject) {
		if (m_pObject != pObject) {
			if (m_pObject)
				m_pObject->DecRefCount();
			m_pObject = pObject;
			if (m_pObject)
				m_pObject->IncRefCount();
		}
		return *this;
	}

	__forceinline bool operator==(T_Data* apObject) const { return (m_pObject == apObject); }

	__forceinline bool operator==(const NiPointer& ptr) const { return (m_pObject == ptr.m_pObject); }

	__forceinline operator bool() const { return m_pObject != nullptr; }
};

struct FogProperties {
	struct FogParameters {
		float fDistFar;
		float fDistNear;
		float fPower;
		float fUnknown;
	};

	FogParameters Parameters;
	NiColorA Color;
};

class NiPixelFormat {
public:
	enum Format {
		FORMAT_RGB = 0,
		FORMAT_RGBA = 1,
		FORMAT_PAL = 2,
		FORMAT_PALALPHA = 3,
		FORMAT_DXT1 = 4,
		FORMAT_DXT3 = 5,
		FORMAT_DXT5 = 6,
		FORMAT_RGB24NONINTERLEAVED = 7,
		FORMAT_BUMP = 8,
		FORMAT_BUMPLUMA = 9,
		FORMAT_RENDERERSPECIFIC = 10,
		FORMAT_ONE_CHANNEL = 11,
		FORMAT_TWO_CHANNEL = 12,
		FORMAT_THREE_CHANNEL = 13,
		FORMAT_FOUR_CHANNEL = 14,
		FORMAT_DEPTH_STENCIL = 15,
		FORMAT_UNKNOWN = 16,
		FORMAT_MAX = 17,
	};

	uint8_t		pad0[2];
	Format		m_eFormat;
	uint32_t	pad1[15];

	bool IsAlpha() const {
		return m_eFormat == FORMAT_DXT3 || m_eFormat == FORMAT_DXT5 || m_eFormat == FORMAT_RGBA;
	}
};

class NiTexture {
public:
	class RendererData {
	public:
		uint32_t		pad0[5];
		NiPixelFormat	m_kPixelFormat;
		uint32_t		pad1;
		uint8_t			pad2;

		bool IsAlphaTexture() const {
			return m_kPixelFormat.IsAlpha();
		}
	};

	uint32_t		pad[9];
	RendererData*	m_pkRendererData;

	bool IsAlphaTexture() const {
		return m_pkRendererData->IsAlphaTexture();
	}
};

class NiTransform {
public:
	uint32_t	pad0[9];
	NiPoint3	m_Translate;
	float		m_fScale;
};

class NiBound {
public:
	NiPoint3	m_kCenter;
	float		m_fRadius;
};

class BSMultiBoundShape {
public:
	enum IntersectResult
	{
		BS_INTERSECT_NONE = 0,
		BS_INTERSECT_PARTIAL = 1,
		BS_INTERSECT_CONTAINSTARGET = 2
	};

	// GAME - 0xC382B0
	// GECK - 0x9E4C40
	IntersectResult	CheckBound(const NiBound& arTargetBound) const {
		// Terrain always has an AABB multibound - we call the AABB variant to avoid the need for a full vftable.
		return ThisCall<IntersectResult>(uiAABBCheckBoundAddress, this, &arTargetBound);
	}
};

class BSMultiBound {
public:
	uint32_t			pad0[3];
	BSMultiBoundShape*	pShape;
};

class BSMultiBoundNode {
public:
	uint32_t		pad0[43];
	BSMultiBound*	pMultiBound;
};

class NiLight {
public:
	uint32_t	pad0[26];
	NiTransform m_kWorld;
	uint32_t	pad1[10];
	float		m_fDimmer;
	NiColor		m_kAmb;
	NiColor		m_kDiff;
	float		m_fRadius;
	uint32_t	pad2[3];

	bool IsLit() {
		DirectX::XMVECTOR kColor = DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&m_kDiff));
		DirectX::XMVECTOR kDimmer = DirectX::XMVectorReplicate(m_fDimmer);
		kColor = DirectX::XMVectorMultiply(kColor, kDimmer);

		DirectX::XMVECTOR kMinComponents = DirectX::XMVectorReplicate(fLightComponentMinValue);

		uint32_t uiCompResult;
		DirectX::XMVectorGreaterR(&uiCompResult, kColor, kMinComponents);
		return DirectX::XMComparisonAnyTrue(uiCompResult);
	}

	bool IsInMultiBound(const BSMultiBoundShape* apShape) {
		if (!apShape)
			return true;

		NiBound kLightBound = { m_kWorld.m_Translate, m_fRadius };
		if (apShape->CheckBound(kLightBound) == BSMultiBoundShape::BS_INTERSECT_NONE)
			return false;

		return true;
	}
};

class NiDirectionalLight : public NiLight {
public:
	NiPoint3 m_kWorldDir;
};

class NiRefObject {
public:
	virtual ~NiRefObject();
	virtual void DeleteThis();

	uint32_t m_uiRefCount;

	// 0x40F6E0
	inline void IncRefCount() {
		InterlockedIncrement(&m_uiRefCount);
	}

	// 0x401970
	inline void DecRefCount() {
		if (!InterlockedDecrement(&m_uiRefCount))
			DeleteThis();
	}
};

class NiObject : public NiRefObject {
public:
	virtual const void*			GetRTTI() const;
	virtual void*				IsNode() const;
	virtual void*				IsFadeNode() const;
	virtual BSMultiBoundNode*	IsMultiBoundNode() const;
};

class NiNode : public NiObject {};

class NiGeometry {
public:
	uint32_t	pad0[6];
	NiNode*		m_pkParent;

	BSMultiBoundShape* GetMultiBoundShape() {
		BSMultiBoundNode* pMBNode = m_pkParent->IsMultiBoundNode();
		
		if (!pMBNode)
			return nullptr;

		if (!pMBNode->pMultiBound)
			return nullptr;

		return pMBNode->pMultiBound->pShape;
	}
};

class NiCamera {
public:
	uint32_t	pad0[26];
	NiTransform m_kWorld;
};

class ShadowSceneLight {
public:
	uint32_t	pad0[52];
	float		fLODDimmer;
	float		fFade;
	uint32_t	pad1[7];
	bool		bPointLight;
	bool		bAmbientLight;
	NiLight*	spLight;
	uint32_t	pad3[85];
};

class ShadowSceneNode {
public:
	uint32_t	pad[121];
	NiPoint3	kLightingOffset;

	static ShadowSceneNode* GetShadowSceneNode(uint32_t aeType) {
		return ((ShadowSceneNode**)uiShadowSceneNodeArrayAddress)[aeType];
	}
};

class NiShaderConstantMapEntry : public NiRefObject {
public:
	bool			bEnabled;
	const char*		mKey;
	uint32_t		pad1[4];
	uint32_t		m_uiRegisterCount;
};

class NiD3DShaderConstantMap {
public:
	// GAME - 0xE81D70
	// GECK - 0xC1AA70
	uint32_t AddEntry(const char* pszKey, uint32_t uiFlags, uint32_t uiExtra, uint32_t uiShaderRegister, uint32_t uiRegisterCount, const char* pszVariableName = "", uint32_t uiDataSize = 0, uint32_t uiDataStride = 0, const void* pvDataSource = 0, bool bCopyData = false) {
		return ThisCall<uint32_t>(uiConstantMapAddEntryAddress, this, pszKey, uiFlags, uiExtra, uiShaderRegister, uiRegisterCount, pszVariableName, uiDataSize, uiDataStride, pvDataSource, bCopyData);
	}

	// GAME - 0xE82640
	// GECK - 0xC1B340
	NiShaderConstantMapEntry* GetEntry(const char* pszKey) const {
		return ThisCall<NiShaderConstantMapEntry*>(uiConstantMapGetEntryAddress, this, pszKey);
	}
};

class BSShaderAccumulator {
public:
	uint32_t	pad0[2];
	NiCamera*	m_pkCamera;
};

class BSShaderProperty {
public:
	class RenderPass {
	public:
		uint32_t			pad0[1];
		uint16_t			usPassEnum;
		uint16_t			pad1;
		uint8_t				pad2;
		uint8_t				ucNumLights;
		uint8_t				ucMaxNumLights;
		uint8_t				cCurrLandTexture;
		ShadowSceneLight**	ppSceneLights;
	};

	class RenderPassArray {
	public:
		uint32_t	 pad0;
		RenderPass** m_pBase;
		uint32_t	 pad1[2];

		uint32_t uiPassCount;

		// GAME - 0xBA9EE0
		// GECK - 0x909B60
		void AddPass(
			void* apGeometry, 
			uint32_t auiPassEnum, 
			bool abFirst,
			uint8_t aucNumLights = 0,
			ShadowSceneLight* apSceneLight = nullptr, 
			ShadowSceneLight* apSceneLight1 = nullptr, 
			ShadowSceneLight* apSceneLight2 = nullptr, 
			ShadowSceneLight* apSceneLight3 = nullptr
		) {
			ThisCall(uiRenderPassArrayAddPassAddress, this, apGeometry, auiPassEnum, abFirst, aucNumLights, apSceneLight, apSceneLight1, apSceneLight2, apSceneLight3);
		}

		RenderPass* GetLastPass() const { return m_pBase[uiPassCount - 1]; }
	};

	uint32_t			pad0[8];
	uint32_t			ulFlags[2];
	uint32_t			pad1[5];
	RenderPassArray*	pRenderPassArray;
	uint32_t			pad2[8];
};

class BSShaderLightingProperty : public BSShaderProperty {
public:
	// GAME - 0xB70390
	// GECK - 0x925430
	void ResortLights(NiBound* apObjBound) {
		ThisCall(uiShaderLightPropResortLights, this, apObjBound);
	}

	// GAME - 0xB70600
	// GECK - 0x9256A0
	ShadowSceneLight* GetFirstActiveNonShadowLight(void** apIter) {
		return ThisCall<ShadowSceneLight*>(uiShaderLightPropGetFirstLightAddress, this, apIter);
	}

	// GAME - 0xB70700
	// GECK - 0x9257A0
	ShadowSceneLight* GetNextActiveNonShadowLight(void** apIter) {
		return ThisCall<ShadowSceneLight*>(uiShaderLightPropGetNextLightAddress, this, apIter);
	}
};

class BSShaderPPLightingProperty : public BSShaderLightingProperty {
public:
	struct SpecularExponents {
		uint8_t ucExponent[10];
	};

	struct SpecularAvailabilities {
		bool bHasSpecular[10];
	};
	
	uint32_t				pad1[3];
	float					fForcedDarkness;
	uint32_t				pad2[14];
	uint16_t				usTextureCount;
	NiTexture**				ppTextures[6];
	SpecularExponents*		pLandSpecularExponents;
	uint16_t				usLandPassCount;
	SpecularAvailabilities* pLandSpecularStatus;

	static NiBound			kCameraBound;

	// GAME - 0xB66640
	// GECK - 0x91F380
	void SetLandscapeSpecularExponents(
		uint8_t aucExp0, 
		uint8_t aucExp1, 
		uint8_t aucExp2, 
		uint8_t aucExp3, 
		uint8_t aucExp4, 
		uint8_t aucExp5, 
		uint8_t aucExp6, 
		uint8_t aucExp7, 
		uint8_t aucExp8, 
		uint8_t aucExp9
	) {
		uint8_t ucExponents[] = { aucExp0, aucExp1, aucExp2, aucExp3, aucExp4, aucExp5, aucExp6, aucExp7, aucExp8, aucExp9 };
		for (uint8_t i = 0; i < usTextureCount; i++) {
			pLandSpecularExponents->ucExponent[i] = ucExponents[i];
		}
	}

	// GAME - 0xBDF790
	// GECK - 0x980BB0
	template <uint32_t uiCall>
	void AddPasses(NiGeometry* apGeometry, uint32_t aeEnabledPasses, uint16_t* apusPassCount, uint32_t aeRenderMode, BSShaderAccumulator* apAccumulator, bool abAddPass) {
		kCameraBound.m_kCenter = apAccumulator->m_pkCamera->m_kWorld.m_Translate;
		kCameraBound.m_fRadius = 70.0f;

		ThisCall(kShaderLightPropAddPasses[uiCall].GetOverwrittenAddr(), this, apGeometry, aeEnabledPasses, apusPassCount, aeRenderMode, apAccumulator, abAddPass);
	}

	// GAME - 0xBDF3E0
	// GECK - 0x980800
	void AddPass_Landscape(NiGeometry* apGeometry, ShadowSceneLight* apSun, uint16_t* apusPassCount, bool abAddPass, bool* abEnable, bool abSpecular, bool abCanopyShadows) {
		ShadowSceneLight* kPointlights[ucMaxPointlights] = {};

		// First, first sort the lights towards the camera.
		ResortLights(&kCameraBound);

		BSMultiBoundShape* pMBShape = apGeometry->GetMultiBoundShape();

		void* kLightIter = nullptr;
		ShadowSceneLight* pLight;
		
		// Get the first active non shadow light that is not "black".
		for (pLight = GetFirstActiveNonShadowLight(&kLightIter); pLight; pLight = GetNextActiveNonShadowLight(&kLightIter)) {
			if (pLight && pLight->spLight->IsLit() && pLight->spLight->IsInMultiBound(pMBShape))
				break;
		}

		kPointlights[0] = pLight;
		uint8_t ucUsedPointlights = pLight ? 1 : 0;

		// Fill the rest of the array with active shadow lights.
		if (ucUsedPointlights < ucMaxPointlights && pLight) {
			for (pLight = GetNextActiveNonShadowLight(&kLightIter); ucUsedPointlights < ucMaxPointlights && pLight; pLight = GetNextActiveNonShadowLight(&kLightIter)) {
				if (pLight && pLight->spLight->IsLit() && pLight->spLight->IsInMultiBound(pMBShape)) {
					kPointlights[ucUsedPointlights] = pLight;
					ucUsedPointlights++;
				}
			}
		}

		uint16_t usPassCount = usLandPassCount >= 7 ? 7 : usLandPassCount;

		bool bSkipNormalMaps = (ulFlags[1] & 0x100000) != 0;

		uint16_t usPassNumber;
		if (ucUsedPointlights) {
			usPassNumber = 8 * usPassCount + 505;
			
			if (abCanopyShadows)
				usPassNumber += 1;

			if (ucUsedPointlights > 6)
				usPassNumber += 2;
			if (ucUsedPointlights > 12)
				usPassNumber += 2;

			if (abAddPass) {
				pRenderPassArray->AddPass(apGeometry, usPassNumber, true, 0);
				BSShaderProperty::RenderPass* pPass = pRenderPassArray->GetLastPass();
				CdeclCall(
					uiRenderPassSetLightsAddress, 
					pPass, 
					uint8_t(1ui8 + ucUsedPointlights),
					apSun, 
					kPointlights[0], 
					kPointlights[1], 
					kPointlights[2], 
					kPointlights[3], 
					kPointlights[4], 
					kPointlights[5],
					kPointlights[6],
					kPointlights[7], 
					kPointlights[8], 
					kPointlights[9], 
					kPointlights[10], 
					kPointlights[11], 
					kPointlights[12], 
					kPointlights[13], 
					kPointlights[14], 
					kPointlights[15], 
					kPointlights[16], 
					kPointlights[17], 
					kPointlights[18],
					kPointlights[19],
					kPointlights[20],
					kPointlights[21],
					kPointlights[22],
					kPointlights[23]
				);
			}
			else
				(*apusPassCount)++;
		}
		else {
			usPassNumber = 8 * usPassCount + 503;

			if (abCanopyShadows)
				usPassNumber += 1;

			if (abAddPass)
				pRenderPassArray->AddPass(apGeometry, usPassNumber, true, 1, apSun);
			else
				(*apusPassCount)++;
		}

		(*abEnable) = false;

		// Close to LOD blending pass.
		if (abAddPass && *((bool*)uiInLODWorldAddress) && (ulFlags[1] & 0x4000) == 0) {
			pRenderPassArray->AddPass(apGeometry, 560U, 0, 1, apSun, 0, 0, 0);
			pRenderPassArray->GetLastPass()->cCurrLandTexture = 9;
		}
		else {
			(*apusPassCount)++;
		}
	}
};

class ShadowLightShader {
public:
	uint32_t				pad0[5];
	LPDIRECT3DDEVICE9		m_pkD3DDevice;
	uint32_t				pad1[6];
	NiD3DShaderConstantMap* m_spPixelConstantMap;

	struct alignas(16) LandSpec {
		float fSpecStatus[8];
	};

	struct alignas(16) LandHeight {
		float fHeightStatus[8];
	};

	static LandSpec				kLandSpec;
	static LandHeight			kLandHeight;
	static float				kLODLandSpec;

	static DirectX::XMVECTOR	kPointlightColors[24];
	static DirectX::XMVECTOR	kPointlightPositions[24];

	static float				fPointlightCount;

	static NiPointer<NiShaderConstantMapEntry> spLandSpecularEntry;
	static NiPointer<NiShaderConstantMapEntry> spLandHeightEntry;
	static NiPointer<NiShaderConstantMapEntry> spStandardFogParamsEntry;
	static NiPointer<NiShaderConstantMapEntry> spStandardFogColorEntry;
	static NiPointer<NiShaderConstantMapEntry> spLandLODSpecEntry;
	static NiPointer<NiShaderConstantMapEntry> spPtlightColorsEntry;
	static NiPointer<NiShaderConstantMapEntry> spPtlightPositionsEntry;
	static NiPointer<NiShaderConstantMapEntry> spPtlightCountEntry;

	static __forceinline uint32_t* const GetVertexConstantFlags() {
		return (uint32_t*)uiSLSVertexConstantFlagsAddress;
	}

	static __forceinline NiShaderConstantMapEntry* GetPixelConstantMapEntry(uint16_t index) {
		return ((NiShaderConstantMapEntry**)uiSLSPixelConstantMapAddress)[index];
	}

	static __forceinline NiShaderConstantMapEntry* GetVertexConstantMapEntry(uint16_t index) {
		return ((NiShaderConstantMapEntry**)uiSLSVertexConstantMapAddress)[index];
	}

	// GAME - 0x11FA280
	// GECK - 0xF28EEC
	static __forceinline FogProperties* const GetFogParam() {
		return (FogProperties*)uiSLSFogParamAddress;
	}

	// GAME - 0x11FA0C0
	// GECK - 0xF244E0
	static __forceinline NiColorA* const GetAmbientColor() {
		return (NiColorA*)uiSLSAmbientColorAddress;
	}

	// GAME - 0x11FA0D0
	// GECK - 0xF244F0
	static __forceinline NiColorA* const GetLightColors() {
		return (NiColorA*)uiSLSLightColorsAddress;
	}

	// GAME - 0x11FD9A8
	// GECK - 0xF28FD0
	static __forceinline NiPoint4* const GetLightPositions() {
		return (NiPoint4*)uiSLSLightPositionsAddress;
	}

	static void EnableEyePositionForLandPasses() {
		// Make sure the land vertex shaders get the EyePosition passed in for view direction calculations.
		GetVertexConstantFlags()[254] |= 1 << 10;
		for (uint32_t i = 503; i < 561; i++) {
			GetVertexConstantFlags()[i] |= 1 << 10;
		}
	}

	// GAME - 0xB7E430
	// GECK - 0x9502A0
	void InitShaderConstants() {
		ThisCall(kSLSInitShaderConstants.GetOverwrittenAddr(), this);

		// Add our custom constants to the pixel shader map.
		constexpr uint32_t uiLightArraySize = ARRAYSIZE(kPointlightColors);
		constexpr uint32_t uiLightArrayDataSize = sizeof(NiPoint4) * uiLightArraySize;
		
		m_spPixelConstantMap->AddEntry("LandSpec",					0x10000004, 0, 32, 2, "", 4, 4, &kLandSpec);
		m_spPixelConstantMap->AddEntry("LandHeight",				0x10000004, 0, 34, 2, "", 4, 4, &kLandHeight);
		m_spPixelConstantMap->AddEntry("StandardFogParams",			0x10000004, 0, 36, 1, "", 4, 4, &GetFogParam()->Parameters);
		m_spPixelConstantMap->AddEntry("StandardFogColor",			0x10000004, 0, 37, 1, "", 4, 4, &GetFogParam()->Color);
		m_spPixelConstantMap->AddEntry("LandLODSpec",				0x10000004, 0, 38, 1, "", 4, 4, &kLODLandSpec);
		m_spPixelConstantMap->AddEntry("PointlightColors",			0x10000007, 0, 39, uiLightArraySize, "", 16, 4, &kPointlightColors);
		m_spPixelConstantMap->AddEntry("PointlightPositions",		0x10000007, 0, 63, uiLightArraySize, "", 16, 4, &kPointlightPositions);
		m_spPixelConstantMap->AddEntry("PointlightCount",			0x10000004, 0, 88, 1, "", 4, 4, &fPointlightCount);
	
		spLandSpecularEntry = m_spPixelConstantMap->GetEntry("LandSpec");
		spLandHeightEntry = m_spPixelConstantMap->GetEntry("LandHeight");
		spStandardFogParamsEntry = m_spPixelConstantMap->GetEntry("StandardFogParams");
		spStandardFogColorEntry = m_spPixelConstantMap->GetEntry("StandardFogColor");
		spLandLODSpecEntry = m_spPixelConstantMap->GetEntry("LandLODSpec");
		spPtlightColorsEntry = m_spPixelConstantMap->GetEntry("PointlightColors");
		spPtlightPositionsEntry = m_spPixelConstantMap->GetEntry("PointlightPositions");
		spPtlightCountEntry = m_spPixelConstantMap->GetEntry("PointlightCount");
	}

	static bool IsLandscapePass(uint32_t aeRenderPassType) {
		return aeRenderPassType >= 503 && aeRenderPassType <= 558;
	}

	static void UpdateLightsAlt(BSShaderPPLightingProperty* apShaderProp, BSShaderProperty::RenderPass* apRenderPass, D3DXMATRIX aMatrix, NiTransform& arTransform, uint32_t aeRenderPassType, void* apSkinInstance) {
		const DirectX::XMVECTOR kLightingOffset = DirectX::XMLoadNiPoint3(ShadowSceneNode::GetShadowSceneNode(0)->kLightingOffset);
		const DirectX::XMMATRIX kMatrix = DirectX::XMLoadD3DXMATRIX(aMatrix);

		bool bSunFound = false;
		uint8_t ucLightNumber = 0;
		uint8_t ucPointLightNumber = 0;
		while (ucLightNumber < apRenderPass->ucNumLights && ucPointLightNumber < ucMaxPointlights) {
			ShadowSceneLight* pSceneLight = apRenderPass->ppSceneLights[ucLightNumber];

			if (!pSceneLight) [[unlikely]] {
				ucLightNumber++;
				continue;
			}

			NiLight* pLight = pSceneLight->spLight;

			if (!pLight || pSceneLight->bAmbientLight) [[unlikely]] {
				ucLightNumber++;
				continue;
			}
			
			bool bPointLight = pSceneLight->bPointLight;

			if (bPointLight) {
				// Point light handling.
				DirectX::XMVECTOR kLightPoint = DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&pLight->m_kWorld.m_Translate));
				kLightPoint = DirectX::XMVectorAdd(kLightPoint, kLightingOffset);
				DirectX::XMVECTOR kFinalPosition = DirectX::XMVector3TransformCoord(kLightPoint, kMatrix);
				kPointlightPositions[ucPointLightNumber] = DirectX::XMVectorSetW(kFinalPosition, pLight->m_fRadius / arTransform.m_fScale);
			}
			else {
				// Directional light handling.
				NiDirectionalLight* pDirLight = static_cast<NiDirectionalLight*>(pLight);
				DirectX::XMVECTOR kSunDir = DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(&pDirLight->m_kWorldDir));
				kSunDir = DirectX::XMVector3Normalize(kSunDir);
				kSunDir = DirectX::XMVectorNegate(kSunDir);
				kSunDir = DirectX::XMVector3TransformNormal(kSunDir, kMatrix);
				kSunDir = DirectX::XMVector3Normalize(kSunDir);
				DirectX::XMStoreNiPoint4(GetLightPositions()[0], DirectX::XMVectorSetW(kSunDir, 1.0f));
			}

			DirectX::XMVECTOR kLightColor = DirectX::XMLoadNiColor(pLight->m_kDiff);
			float fDimmer = pLight->m_fDimmer;

			// Non HDR.
			if (!*(bool*)uiHDRAddress && fDimmer > 1.0f) {
				fDimmer = 1.0f;
			}

			fDimmer *= apShaderProp->fForcedDarkness;
			fDimmer *= pSceneLight->fLODDimmer;
			
			if (bPointLight) {
				if (apShaderProp->fForcedDarkness < 1.0) [[unlikely]]
					fDimmer = 0.0f;
			}
			else if (*(bool*)uiHDRAddress && !*(bool*)uiInInteriorAddress) {
				fDimmer *= *(float*)uiSunlightDimmerAddress;
			}

			kLightColor = DirectX::XMVectorScale(kLightColor, fDimmer);

			if (bPointLight) {
				kPointlightColors[ucPointLightNumber] = DirectX::XMVectorSetW(kLightColor, pSceneLight->fFade);
				ucPointLightNumber++;
			}
			else {
				DirectX::XMStoreNiColorA(GetLightColors()[0], DirectX::XMVectorSetW(kLightColor, pSceneLight->fFade));

				float fDimmer = pLight->m_fDimmer;

				// Non HDR.
				if (!*(bool*)uiHDRAddress && fDimmer > 1.0f) {
					fDimmer = 1.0f;
				}

				NiColor kAmbientColor = pLight->m_kAmb;
				kAmbientColor *= fDimmer;
				float fMagicNightEyeAmbient = *(float*)uiMagicNightEyeAmbientAddress;
				if (fMagicNightEyeAmbient > 0.f) {
					float fLuminance = kAmbientColor.r * 0.33 + kAmbientColor.g * 0.34 + kAmbientColor.b + 0.33;
					if (fLuminance > 0.f) {
						float fLumMult = 1.f;
						if (fLuminance <= fMagicNightEyeAmbient)
							fLumMult = fMagicNightEyeAmbient / fLuminance;
						kAmbientColor = (kAmbientColor + 0.1f) * fLumMult;
					}
					else {
						kAmbientColor = fMagicNightEyeAmbient;
					}
				}
				kAmbientColor *= apShaderProp->fForcedDarkness;
				*GetAmbientColor() = kAmbientColor;

				bSunFound = true;
			}

			ucLightNumber++;
		}

		fPointlightCount = ucPointLightNumber;

		if (bSunFound) {
			GetPixelConstantMapEntry(2)->m_uiRegisterCount = 1;
			GetPixelConstantMapEntry(7)->m_uiRegisterCount = 1;
		}

		if (GetPixelConstantMapEntry(2)->bEnabled && spPtlightColorsEntry) {
			spPtlightColorsEntry->bEnabled = true;
			spPtlightColorsEntry->m_uiRegisterCount = ucPointLightNumber;
			if (spPtlightCountEntry) {
				spPtlightCountEntry->bEnabled = true;
			}
		}

		if (GetPixelConstantMapEntry(7)->bEnabled && spPtlightPositionsEntry) {
			spPtlightPositionsEntry->bEnabled = true;
			spPtlightPositionsEntry->m_uiRegisterCount = ucPointLightNumber;
		}
	}

	// GAME - 0xB78A90
	// GECK - 0x94ACE0
	void UpdateLights(BSShaderPPLightingProperty* apShaderProp, BSShaderProperty::RenderPass* apRenderPass, D3DXMATRIX aMatrix, NiTransform& arTransform, uint32_t aeRenderPassType, void* apSkinInstance) {
		// Update lights doesn't work well with higher light counts in a single pass - positions and colors are only updated up to 8 lights total.
		// Moreover, pixel shader light position constants can only go up to 8 pointlights to avoid clashing with Toggles at c27 (c19-c26).
		if (IsLandscapePass(aeRenderPassType)) [[unlikely]]
			UpdateLightsAlt(apShaderProp, apRenderPass, aMatrix, arTransform, aeRenderPassType, apSkinInstance);
		else
			ThisCall(kSLSUpdateLights.GetOverwrittenAddr(), this, apShaderProp, apRenderPass, aMatrix, &arTransform, aeRenderPassType, apSkinInstance);
	}

	// GAME - 0xB795B0
	// GECK - 0x94B800
	void UpdateToggles(uint32_t aeRenderPassType, void* apGeo, BSShaderPPLightingProperty* apShaderProp, void* apMatProp, BSShaderProperty::RenderPass* apRenderPass, void* apAlphaProp) {
		// Call the original method first.
		ThisCall(kSLSUpdateToggles.GetOverwrittenAddr(), this, aeRenderPassType, apGeo, apShaderProp, apMatProp, apRenderPass, apAlphaProp);

		if (spLandSpecularEntry)
			spLandSpecularEntry->bEnabled = false;
		if (spLandHeightEntry)
			spLandHeightEntry->bEnabled = false;
		if (spStandardFogParamsEntry)
			spStandardFogParamsEntry->bEnabled = false;
		if (spStandardFogColorEntry)
			spStandardFogColorEntry->bEnabled = false;
		if (spLandLODSpecEntry)
			spLandLODSpecEntry->bEnabled = false;

		// Set specific constants for land and land LOD passes.
		if (aeRenderPassType == 254) {
			// ADT Modelspace normals (used for LOD terrain).
			bool bUseSpecular = false;
			for (uint16_t i = 0; i < apShaderProp->usTextureCount; i++) {
				NiTexture* pNormalMap = apShaderProp->ppTextures[1][i];
				if (pNormalMap) {
					bool bIsAlpha = pNormalMap->IsAlphaTexture();
					bUseSpecular |= bIsAlpha;
				}
			}

			kLODLandSpec = bUseSpecular * fLODLandShine;

			if (spLandLODSpecEntry)
				spLandLODSpecEntry->bEnabled = true;
		}
		else if (aeRenderPassType >= 503 && aeRenderPassType <= 558) {
			// Landscape.
			for (uint16_t i = 0; i < 8; i++) {
				kLandSpec.fSpecStatus[i] = apShaderProp->pLandSpecularStatus->bHasSpecular[i] * apShaderProp->pLandSpecularExponents->ucExponent[i];
				
				NiTexture* pDiffuseMap = apShaderProp->ppTextures[0][i];
				kLandHeight.fHeightStatus[i] = pDiffuseMap ? pDiffuseMap->IsAlphaTexture() : 0.0f;
			}

			if (spLandSpecularEntry)
				spLandSpecularEntry->bEnabled = true;
			if (spLandHeightEntry)
				spLandHeightEntry->bEnabled = true;
			if (spStandardFogParamsEntry)
				spStandardFogParamsEntry->bEnabled = true;
			if (spStandardFogColorEntry)
				spStandardFogColorEntry->bEnabled = true;
		}
		else if (aeRenderPassType == 560) {
			// LANDLO (landscape-LOD fade).
			NiTexture* pNormalTex = nullptr;
			if (apRenderPass->cCurrLandTexture < apShaderProp->usTextureCount)
				pNormalTex = apShaderProp->ppTextures[1][apRenderPass->cCurrLandTexture];

			kLODLandSpec = pNormalTex ? pNormalTex->IsAlphaTexture() * fLODLandShine : 0.f;
			
			if (spLandLODSpecEntry)
				spLandLODSpecEntry->bEnabled = true;
		}
	}

	// GAME - 0xB7A730
	// GECK - 0x94C750
	bool LoadStagesAndPasses() {
		bool res = ThisCall<bool>(kSLSLoadStagesAndPasses.GetOverwrittenAddr(), this);
		EnableEyePositionForLandPasses();
		return res;
	}

	// GAME - 0xB78980
	// GECK - 0x94ABD0
	void Reinitialize() {
		ThisCall(kSLSReinitialize.GetOverwrittenAddr(), this);
		EnableEyePositionForLandPasses();
	}
};

NiBound BSShaderPPLightingProperty::kCameraBound = {};
ShadowLightShader::LandSpec ShadowLightShader::kLandSpec = {};
ShadowLightShader::LandHeight ShadowLightShader::kLandHeight = {};
float ShadowLightShader::kLODLandSpec = 0.0f;
DirectX::XMVECTOR ShadowLightShader::kPointlightColors[24] = {};
DirectX::XMVECTOR ShadowLightShader::kPointlightPositions[24] = {};
float ShadowLightShader::fPointlightCount = 0;
NiPointer<NiShaderConstantMapEntry> ShadowLightShader::spLandSpecularEntry;
NiPointer<NiShaderConstantMapEntry> ShadowLightShader::spLandHeightEntry;
NiPointer<NiShaderConstantMapEntry> ShadowLightShader::spStandardFogParamsEntry;
NiPointer<NiShaderConstantMapEntry> ShadowLightShader::spStandardFogColorEntry;
NiPointer<NiShaderConstantMapEntry> ShadowLightShader::spLandLODSpecEntry;
NiPointer<NiShaderConstantMapEntry> ShadowLightShader::spPtlightColorsEntry;
NiPointer<NiShaderConstantMapEntry> ShadowLightShader::spPtlightPositionsEntry;
NiPointer<NiShaderConstantMapEntry> ShadowLightShader::spPtlightCountEntry;

void InitHooks(bool abGECK) {
	// BSMultiBoundAABB
	uiAABBCheckBoundAddress = abGECK ? 0x9E4C40 : 0xC382B0;

	// ShadowSceneNode
	uiShadowSceneNodeArrayAddress = abGECK ? 0xF23C18 : 0x11F91C8;

	// NiD3DShaderConstantMap
	uiConstantMapAddEntryAddress = abGECK ? 0xC1AA70 : 0xE81D70;
	uiConstantMapGetEntryAddress = abGECK ? 0xC1B340 : 0xE82640;

	// ShadowLightShader
	uiSLSVertexConstantFlagsAddress = abGECK ? 0xF282A8 : 0x11FCC80;
	uiSLSFogParamAddress = abGECK ? 0xF246A0 : 0x11FA280;
	uiSLSAmbientColorAddress = abGECK ? 0xF244E0 : 0x11FA0C0;
	uiSLSLightPositionsAddress = abGECK ? 0xF28FD0 : 0x11FD9A8;
	uiSLSLightColorsAddress = abGECK ? 0xF244F0 : 0x11FA0D0;
	uiSLSPixelConstantMapAddress = abGECK ? 0xF2A258 : 0x11FEC30;
	uiSLSVertexConstantMapAddress = abGECK ? 0xF2A200 : 0x11FEBD8;

	uiMagicNightEyeAmbientAddress = abGECK ? 0xF23ECC : 0x11F947C;
	uiInInteriorAddress = abGECK ? 0xF23E77 : 0x11F9427;
	uiSunlightDimmerAddress = abGECK ? 0xF23BE0 : 0x11F9190;

	// RenderPass
	uiRenderPassSetLightsAddress = abGECK ? 0x908950 : 0xBA8C50;

	// RenderPassArray
	uiRenderPassArrayAddPassAddress = abGECK ? 0x909B60 : 0xBA9EE0;

	// BSShaderLightingProperty
	uiShaderLightPropResortLights = abGECK ? 0x925430 : 0xB70390;
	uiShaderLightPropGetFirstLightAddress = abGECK ? 0x9256A0 : 0xB70600;
	uiShaderLightPropGetNextLightAddress = abGECK ? 0x9257A0 : 0xB70700;

	// BSShaderManager
	uiCameraPosAddress = abGECK ? 0xF2014C : 0x11F474C;
	uiInLODWorldAddress = abGECK ? 0xF23EFB : 0x11F94AB;
	uiHDRAddress = abGECK ? 0xF23E6E : 0x11F941E;

	// BSShaderPPLightingProperty::SetLandscapeSpecularExponents hook, to actually fetch specular exponents.
	WriteRelJumpEx(abGECK ? 0x91F380 : 0xB66640, &BSShaderPPLightingProperty::SetLandscapeSpecularExponents);

	// BSShaderPPLightingProperty::AddPasses
	kShaderLightPropAddPasses[0].ReplaceVirtualFuncEx(abGECK ? 0xDD5E78 : 0x10AE1E8, &BSShaderPPLightingProperty::AddPasses<0>);
	kShaderLightPropAddPasses[1].ReplaceVirtualFuncEx(abGECK ? 0xDD6CD8 : 0x10B8448, &BSShaderPPLightingProperty::AddPasses<1>);
	kShaderLightPropAddPasses[2].ReplaceVirtualFuncEx(abGECK ? 0xDD8778 : 0x10B9450, &BSShaderPPLightingProperty::AddPasses<2>);
	kShaderLightPropAddPasses[3].ReplaceVirtualFuncEx(abGECK ? 0xDD89E0 : 0x10B95A8, &BSShaderPPLightingProperty::AddPasses<3>);
	kShaderLightPropAddPasses[4].ReplaceVirtualFuncEx(abGECK ? 0xDD8C18 : 0x10B9A28, &BSShaderPPLightingProperty::AddPasses<4>);
	kShaderLightPropAddPasses[5].ReplaceVirtualFuncEx(abGECK ? 0xDD95D8 : 0x10BAD10, &BSShaderPPLightingProperty::AddPasses<5>);
	kShaderLightPropAddPasses[6].ReplaceVirtualFuncEx(abGECK ? 0xDDBE80 : 0x10BCC78, &BSShaderPPLightingProperty::AddPasses<6>);

	// BSShaderPPLightingProperty::AddPass::Landscape hook.
	WriteRelJumpEx(abGECK ? 0x980800 : 0xBDF3E0, &BSShaderPPLightingProperty::AddPass_Landscape);

	// ShadowLightShader::InitShaderConstants hook.
	kSLSInitShaderConstants.ReplaceVirtualFuncEx(abGECK ? 0xDD82E4 : 0x10AF414, &ShadowLightShader::InitShaderConstants);

	// ShadowLightShader::UpdateLights detour.
	kSLSUpdateLights.ReplaceCallEx(abGECK ? 0x94FB9C : 0xB7DBAC, &ShadowLightShader::UpdateLights);

	// ShadowLightShader::UpdateToggles detour.
	kSLSUpdateToggles.ReplaceCallEx(abGECK ? 0x94FC4B : 0xB7DC5B, &ShadowLightShader::UpdateToggles);

	// ShadowLightShader::LoadStagesAndPasses hook.
	kSLSLoadStagesAndPasses.ReplaceVirtualFuncEx(abGECK ? 0xDD8318 : 0x10AF448, &ShadowLightShader::LoadStagesAndPasses);

	// ShadowLightShader::Reinitialize hook.
	kSLSReinitialize.ReplaceVirtualFuncEx(abGECK ? 0xDD82E8 : 0x10AF418, &ShadowLightShader::Reinitialize);
}

EXTERN_DLL_EXPORT bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info) {
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "Vanilla Plus Terrain";
	info->version = 100;
	return true;
}

EXTERN_DLL_EXPORT bool NVSEPlugin_Load(NVSEInterface* nvse) {
	HMODULE hShaderLoader = GetModuleHandle("Fallout Shader Loader.dll");
	HMODULE hLODFlickerFix = GetModuleHandle("LODFlickerFix.dll");
	HMODULE hILS = GetModuleHandle("ImprovedLightingShaders.dll");

	if (hILS) {
		MessageBox(NULL, "Improved Lighting Shaders found.\nVanilla Plus Terrain is not compatible with ILS, please disable it.", "Vanilla Plus Terrain", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	if (!hShaderLoader) {
		MessageBox(NULL, "Fallout Shader Loader not found.\nVanilla Plus Terrain cannot be used without it, please install it.", "Vanilla Plus Terrain", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	if (!hLODFlickerFix) {
		MessageBox(NULL, "LOD Flicker Fix not found.\nVanilla Plus Terrain cannot be used without it, please install it.", "Vanilla Plus Terrain", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	auto pQuery = (_NVSEPlugin_Query)GetProcAddress(hShaderLoader, "NVSEPlugin_Query");
	PluginInfo kInfo = {};
	pQuery(nvse, &kInfo);
	if (kInfo.version < uiShaderLoaderVersion) {
		char cBuffer[192];
		sprintf_s(cBuffer, "Fallout Shader Loader is outdated.\nPlease update it to use Vanilla Plus Terrain!\nCurrent version: %i\nMinimum required version: %i", kInfo.version, uiShaderLoaderVersion);
		MessageBox(NULL, cBuffer, "Vanilla Plus Terrain", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}
	else {
		InitHooks(nvse->isEditor);
	}

	return true;
}

BOOL WINAPI DllMain(
	HANDLE  hDllHandle,
	DWORD   dwReason,
	LPVOID  lpreserved
)
{
	return TRUE;
}