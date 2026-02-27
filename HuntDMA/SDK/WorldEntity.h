#pragma once
#include "Memory.h"
#include <chrono>
#include <string>
#include <regex>
struct RenderNode {
	char pad_01[0x10]; // 0x00(0x10)
	unsigned int rnd_flags; // 0x10(0x04) — was uint64, changed to uint32 to avoid corrupting +0x14
	char pad_02[0x11C]; // 0x14(0x11C)
	unsigned int silhouettes_param; // 0x130(0x04) — moved from 0x2C
};
struct HealthBar {
	char pad_01[0x18]; // 0x00(0x18)
	unsigned int current_hp; // 0x18(0x04)
	unsigned int regenerable_max_hp; // 0x1C(0x04)
	unsigned int current_max_hp; // 0x20(0x04)
};
struct EntityNameStruct {
	char name[100];
};
enum class EntityType : int
{
	Unknown,

	LocalPlayer,
	EnemyPlayer,
	FriendlyPlayer,
	DeadPlayer,

	Butcher,
	Spider,
	Assassin,
	Scrapbeak,
	Rotjaw,
	Hellborn,

	CashRegister,
	GoldCashRegister,

	Pouch,
	Poster,
	WorkbenchUpgrade,
	WorkbenchOil,
	Trait,
	SealedTrait,

	ResupplyStation,
	ExtractionPoint,
	Clue,

	BearTrap,
	TripMine,
	DarksightDynamite,

	OilBarrel,
	GunpowderBarrel,
	BioBarrel,

	AmmoSwapBox,
	SpitzerBullets,
	PoisonBullets,
	HighVelocityBullets,
	IncendiaryBullets,
	DumDumBullets,
	ExplosiveBullets,
	FMJBullets,
	PoisonBolts,
	ExplosiveBolts,
	IncendiaryShells,
	PennyShotShells,
	FlechetteShells,
	SlugShells,

	SupplyBox,
	AmmoCrate,
	SpecialAmmoCrate,
	CompactAmmoPackage,
	MediumAmmoPackage,
	LongAmmoPackage,
	ShotgunAmmoPackage,
	Medkit,

	Event,
	EventBoon,
};
struct Matrix34 {
	// CryEngine Matrix34: 3 rows × 4 cols (row-major)
	// row0 = [m00 m01 m02 m03]  (X-axis + Tx)
	// row1 = [m10 m11 m12 m13]  (Y-axis + Ty)
	// row2 = [m20 m21 m22 m23]  (Z-axis + Tz)
	float m[3][4];

	Matrix34() { memset(m, 0, sizeof(m)); }

	// Transform local bone offset → world space
	Vector3 TransformPoint(const Vector3& p) const {
		return Vector3(
			m[0][0]*p.x + m[0][1]*p.y + m[0][2]*p.z + m[0][3],
			m[1][0]*p.x + m[1][1]*p.y + m[1][2]*p.z + m[1][3],
			m[2][0]*p.x + m[2][1]*p.y + m[2][2]*p.z + m[2][3]
		);
	}
};

// Keep Matrix4x4 alias for compatibility
using Matrix4x4 = Matrix34;

// ── Double-buffer snapshot for render thread ──────────────────────────────
// DMA thread writes to live fields, then CommitRenderData() copies them here.
// Render thread reads ONLY from this struct (via ent->Render.*) — no torn reads.
// Weapon code → short in-game name lookup
inline std::string ParseWeaponName(const std::string& className) {
	// Extract weapon code (e.g. "WR0008" from "2rRifleBoltClipWR0008MosinNagantM1891")
	for (size_t i = 0; i < className.size(); i++) {
		if (className[i] == 'W' && i + 5 < className.size()) {
			char t = className[i+1];
			if ((t == 'P' || t == 'R' || t == 'S' || t == 'M' || t == 'T' || t == 'C' || t == 'W' || t == 'p') &&
				isdigit(className[i+2]) && isdigit(className[i+3]) && isdigit(className[i+4]) && isdigit(className[i+5])) {
				std::string code = className.substr(i, 6); // e.g. "WR0008"
				// Normalize lowercase 'p' → 'P'
				if (code[1] == 'p') code[1] = 'P';

				static const std::unordered_map<std::string, std::string> weaponMap = {
					// ── Pistols (WP) ──
					{"WP0002", "Dolch 96"},
					{"WP0004", "Officer Nagant"},
					{"WP0007", "Chain Pistol"},
					{"WP0008", "Nagant Precision"},
					{"WP0009", "LeMat Mark II"},
					{"WP0012", "Hand Crossbow"},
					{"WP0014", "Uppercut"},
					{"WP0033", "Scottfield Snubnose"},
					{"WP0035", "Pax"},
					{"WP0037", "New Army"},
					{"WP0044", "Bornheim No.3"},
					// ── Rifles (WR) ──
					{"WR0004", "Winfield Silencer"},
					{"WR0005", "Winfield Marksman"},
					{"WR0008", "Mosin-Nagant"},
					{"WR0009", "Mosin-Nagant Sniper"},
					{"WR0010", "Winfield Swift"},
					{"WR0011", "Vetterli Silencer"},
					{"WR0013", "Vetterli Deadeye"},
					{"WR0016", "Avtomat"},
					{"WR0017", "Nitro Express"},
					{"WR0020", "Crossbow"},
					{"WR0022", "Sparks LRR"},
					{"WR0025", "Winfield Aperture"},
					{"WR0029", "Officer Carbine"},
					{"WR0034", "Mosin Obrez"},
					{"WR0035", "Lebel Marksman"},
					{"WR0037", "Lebel Aperture"},
					{"WR0050", "Winfield Vandal"},
					{"WR0051", "Winfield Vandal Deadeye"},
					{"WR0053", "Centennial"},
					{"WR0065", "Officer Carbine Deadeye"},
					{"WR0068", "Berthier"},
					{"WR0071", "Krag"},
					{"WR0072", "Centennial Shorty"},
					{"WR0075", "LeMat Carbine"},
					{"WR0078", "Krag Sniper"},
					{"WR0080", "Drilling"},
					{"WR0082", "Drilling Handcannon"},
					{"WR0085", "Marlin"},
					{"WR0088", "Marathon"},
					// ── Shotguns (WS) ──
					{"WS0001", "Caldwell Rival"},
					{"WS0002", "Specter"},
					{"WS0006", "Crown & King"},
					{"WS0016", "Terminus"},
					{"WS0018", "Caldwell Slate"},
					{"WS0024", "Rival Handcannon"},
					// ── Melee (WM) ──
					{"WM0000", "Fists"},
					{"WM0001", "Knife"},
					{"WM0002", "Dusters"},
					{"WM0003", "Heavy Knife"},
					{"WM0004", "Bomb Lance"},
					// ── Tools (WT) ──
					{"WT0005", "Throwing Knives"},
					{"WT0009", "Derringer"},
					{"WT0010", "Spyglass"},
					{"WT0019", "Throwing Axes"},
					{"WT0020", "Choke Bombs"},
					{"WT0023", "Spear"},
					// ── Consumables (WC) ──
					{"WC0001", "Frag Bomb"},
					{"WC0005", "Sticky Bomb"},
					{"WC0007", "Fire Bomb"},
					{"WC0009", "Concertina Bomb"},
					{"WC0010", "Dynamite Bundle"},
					{"WC0011", "Dynamite Stick"},
					{"WC0012", "Big Dynamite Bundle"},
					{"WC0013", "Poison Bomb"},
					{"WC0018", "Waxed Dynamite"},
					{"WC0020", "Hellfire"},
					{"WC0035", "Beetle (Fire)"},
					{"WC0036", "Beetle (Shrapnel)"},
					{"WC0039", "Beetle (Choke)"},
					// ── World items (WW) ──
					{"WW0001", "Sledgehammer"},
					{"WW0003", "Axe"},
					{"WW0005", "Lantern"},
					{"WW0008", "Bear Trap"},
					{"WW0009", "Shovel"},
					{"WW0010", "Pitchfork"},
					{"WW0017", "Powder Keg"},
					// ── Additional Pistols ──
					{"WP0001", "Nagant M1895"},
					{"WP0003", "Bornheim No.3"},
					{"WP0005", "Scottfield"},
					{"WP0006", "Caldwell Conversion"},
					{"WP0010", "Caldwell Pax"},
					{"WP0011", "Sparks Pistol"},
					{"WP0013", "Scottfield Spitfire"},
					{"WP0015", "Bornheim Extended"},
					// ── Additional Rifles ──
					{"WR0001", "Winfield M1873C"},
					{"WR0002", "Winfield M1873C"},
					{"WR0003", "Winfield M1873C"},
					{"WR0006", "Vetterli 71"},
					{"WR0007", "Vetterli 71"},
					{"WR0012", "Springfield 1866"},
					{"WR0014", "Springfield Compact"},
					{"WR0015", "Springfield Marksman"},
					{"WR0018", "Springfield Deadeye"},
					{"WR0019", "Springfield 1866"},
					{"WR0021", "Sparks LRR Silencer"},
					{"WR0023", "Sparks LRR Sniper"},
					{"WR0024", "Winfield M1873C"},
					{"WR0026", "Lebel 1886"},
					{"WR0027", "Lebel 1886"},
					{"WR0028", "Nagant Carbine"},
					{"WR0030", "Vetterli Marksman"},
					{"WR0031", "Springfield Striker"},
					{"WR0032", "Centennial"},
					{"WR0033", "Mosin-Nagant Bayonet"},
					{"WR0036", "Lebel 1886"},
					{"WR0038", "Winfield Musket"},
					{"WR0040", "Bow"},
					// ── Additional Shotguns ──
					{"WS0003", "Specter Bayonet"},
					{"WS0004", "Specter Compact"},
					{"WS0005", "Romero 77"},
					{"WS0007", "Romero Handcannon"},
					{"WS0008", "Romero Hatchet"},
					{"WS0009", "Romero 77"},
					{"WS0010", "Romero Alamo"},
					{"WS0011", "Romero Starshell"},
					{"WS0012", "Romero Talon"},
					{"WS0013", "LeMat Shotgun"},
					{"WS0014", "Specter 1882"},
					{"WS0015", "Rival 78"},
					{"WS0017", "Slate"},
					// ── Additional Melee ──
					{"WM0005", "Machete"},
					{"WM0006", "Cavalry Saber"},
					{"WM0007", "Combat Axe"},
				};

				auto it = weaponMap.find(code);
				if (it != weaponMap.end())
					return it->second;

				// Fallback: truncate after last digit to strip skin names, then CamelCase
				std::string raw = className.substr(i + 6);
				size_t lastDigit = std::string::npos;
				for (size_t k = 0; k < raw.size(); k++) {
					if (isdigit(raw[k])) lastDigit = k;
				}
				if (lastDigit != std::string::npos && lastDigit + 1 < raw.size())
					raw = raw.substr(0, lastDigit + 1);
				std::string result;
				for (size_t j = 0; j < raw.size(); j++) {
					if (j > 0 && isupper(raw[j]) && !isupper(raw[j-1]))
						result += ' ';
					else if (j > 0 && isupper(raw[j]) && isupper(raw[j-1]) && j+1 < raw.size() && islower(raw[j+1]))
						result += ' ';
					result += raw[j];
				}
				return result;
			}
		}
	}
	return className;
}

struct EntityRenderData {
	static const int MAX_BONES = 15;
	Vector3    Position;
	Vector3    BonePositions[MAX_BONES];
	Vector3    HeadPosition;
	HealthBar  Health;
	RenderNode Node;
	EntityType Type       = EntityType::Unknown;
	uint32_t   InternalFlags = 0;
	bool       Valid      = true;
	bool       Hidden     = false;
	std::string WeaponName1;  // Primary weapon
	std::string WeaponName2;  // Secondary weapon
};

class WorldEntity
{
private:
	uint64_t Class = 0x0;
	uint64_t ClassAddress = 0x0;
	const uint64_t PosOffset = 0x134;
	const uint64_t StringBufferOffset = 0x10;
	const uint64_t ClassPointerOffset = 0x18;
	const uint64_t SlotsPointerOffset = 0xA8;
	const uint64_t SlotOffset = 0x0;
	const uint64_t RenderNodePointerOffset = 0xA0;
	RenderNode Node;
	EntityNameStruct EntityName;
	EntityNameStruct ClassName;

	uint64_t EntityNamePointer = 0x0;
	uint64_t ClassNamePointer = 0x0;
	uint64_t ClassPointer = 0x0;
	uint64_t SlotsPointer = 0x0;
	uint64_t Slot = 0x0;
	uint64_t RenderNodePointer = 0x0;
	Vector3 Position;
	EntityType Type;

	const uint64_t TypeNameOffset1 = 0x20;
	const uint64_t TypeNameOffset2 = 0x8;
	uint64_t TypeNamePointer1 = 0x0;
	uint64_t TypeNamePointer2 = 0x0;
	EntityNameStruct TypeName;

	const uint64_t HpOffset1 = 0x198;
	const uint64_t HpOffset2 = 0x20;
	const uint64_t HpOffset3 = 0xC8;
	const uint64_t HpOffset4 = 0x78;
	const uint64_t HpOffset5 = 0x58;
	uint64_t HpPointer1 = 0x0;
	uint64_t HpPointer2 = 0x0;
	uint64_t HpPointer3 = 0x0;
	uint64_t HpPointer4 = 0x0;
	uint64_t HpPointer5 = 0x0;
	HealthBar Health;

	const uint64_t CharacterInstanceOffset = 0x88;   // Slot -> CCharInstance*
	const uint64_t SkeletonPoseOffset       = 0xC80;  // CCharInstance -> CSkeletonPose (inline)
	const uint64_t BoneArrayBaseOffset      = 0x20;   // CSkeletonPose -> QuatT array base ptr
	const uint64_t BoneArrayAlignOffset     = 0x38;   // CSkeletonPose -> QuatT array align ptr
	const uint64_t DefaultSkeletonOffset    = 0x1C0;  // CCharInstance -> IDefaultSkeleton ptr
	const uint64_t ModelJointsOffset        = 0x8;    // IDefaultSkeleton -> joint name array
	const uint64_t BoneArraySizeOffset      = 0xA0;   // IDefaultSkeleton -> joint count
	const uint64_t BoneStructSize           = 0x1C;   // sizeof(QuatT)
	const uint64_t BonePosOffset            = 0x10;   // QuatT -> Vec3 position
	const uint64_t WorldMatrixOffset        = 0x160;
	static const int MAX_BONES = 15;
	uint32_t BoneCount = 0;
	int BoneIndex[MAX_BONES] = { 0 };
	Vector3 HeadPosition;
	Vector3 BonePositions[MAX_BONES];  // world-space positions, updated every frame
	Vector3 LocalBonePositions[MAX_BONES];  // raw local-space from scatter, temp buffer
	uint64_t HeadBonePtr = 0;  // cached ptr to head QuatT — valid after UpdateBones()
	std::string DebugBoneNames;  // first few bone names from skeleton — for offset debugging
	bool BonesMapped = false;    // true once UpdateBones() has built the bone index map
	Matrix4x4 WorldMatrix;       // entity world transform — for bone local->world transform

	const uint64_t InternalFlagsOffset = 0x8;
	uint32_t InternalFlags = 0x0;
	bool Hidden = false;
	std::string WeaponName1;  // primary weapon (set during CacheEntities)
	std::string WeaponName2;  // secondary weapon

	bool Valid = true;

	std::unordered_map<EntityType, std::string> Names = {
		{EntityType::Unknown, "Unknown"},

		{EntityType::LocalPlayer, "LocalPlayer"},
		{EntityType::EnemyPlayer, "EnemyPlayer"},
		{EntityType::FriendlyPlayer, "FriendlyPlayer"},
		{EntityType::DeadPlayer, "DeadPlayer"},

		{EntityType::Butcher, "Butcher"},
		{EntityType::Assassin, "Assassin"},
		{EntityType::Scrapbeak, "Scrapbeak"},
		{EntityType::Spider, "Spider"},
		{EntityType::Rotjaw, "Rotjaw"},
		{EntityType::Hellborn, "Hellborn"},

		{EntityType::CashRegister, "CashRegister"},
		{EntityType::GoldCashRegister, "GoldCashRegister"},

		{EntityType::Pouch, "Pouch"},
		{EntityType::Poster, "Poster"},
		{EntityType::WorkbenchUpgrade, "WorkbenchUpgrade"},
		{EntityType::WorkbenchOil, "WorkbenchOil"},
		{EntityType::Trait, "Trait"},
		{EntityType::SealedTrait, "Sealed Trait"},

		{EntityType::ResupplyStation, "ResupplyStation"},
		{EntityType::ExtractionPoint, "ExtractionPoint"},
		{EntityType::Clue, "Clue"},

		{EntityType::BearTrap, "BearTrap"},
		{EntityType::TripMine, "TripMine"},
		{EntityType::DarksightDynamite, "DarksightDynamite"},

		{EntityType::OilBarrel, "OilBarrel"},
		{EntityType::GunpowderBarrel, "GunpowderBarrel"},
		{EntityType::BioBarrel, "BioBarrel"},

		{EntityType::AmmoSwapBox, "AmmoSwapBox"},
		{EntityType::SpitzerBullets, "SpitzerBullets"},
		{EntityType::PoisonBullets, "PoisonBullets"},
		{EntityType::HighVelocityBullets, "HighVelocityBullets"},
		{EntityType::IncendiaryBullets, "IncendiaryBullets"},
		{EntityType::DumDumBullets, "DumDumBullets"},
		{EntityType::ExplosiveBullets, "ExplosiveBullets"},
		{EntityType::FMJBullets, "FMJBullets"},
		{EntityType::PoisonBolts, "PoisonBolts"},
		{EntityType::ExplosiveBolts, "ExplosiveBolts"},
		{EntityType::IncendiaryShells, "IncendiaryShells"},
		{EntityType::PennyShotShells, "PennyShotShells"},
		{EntityType::FlechetteShells, "FlechetteShells"},
		{EntityType::SlugShells, "SlugShells"},

		{EntityType::SupplyBox, "SupplyBox"},
		{EntityType::AmmoCrate, "AmmoCrate"},
		{EntityType::SpecialAmmoCrate, "SpecialAmmoCrate"},
		{EntityType::CompactAmmoPackage, "CompactAmmoPackage"},
		{EntityType::MediumAmmoPackage, "MediumAmmoPackage"},
		{EntityType::LongAmmoPackage, "LongAmmoPackage"},
		{EntityType::ShotgunAmmoPackage, "ShotgunAmmoPackage"},
		{EntityType::Medkit, "Medkit"},

		{EntityType::Event, "Event"},
		{EntityType::EventBoon, "Event Boon"},
	};
public:
	WorldEntity(uint64_t classptr);
	void SetUp(VMMDLL_SCATTER_HANDLE handle);
	void SetUp1(VMMDLL_SCATTER_HANDLE handle);
	void SetUp2(VMMDLL_SCATTER_HANDLE handle);
	void SetUp3(VMMDLL_SCATTER_HANDLE handle);
	void SetUp4(VMMDLL_SCATTER_HANDLE handle);
	EntityNameStruct GetEntityName() { return EntityName; }
	EntityNameStruct GetEntityClassName() { return ClassName; }
	EntityNameStruct GetTypeName() { return TypeName; }
	std::string CompactTypeName = "";
	Vector3 GetPosition() { return Position; }
	RenderNode GetRenderNode() { return Node; }
	HealthBar GetHealth() { return Health; }
	uint32_t GetInternalFlags() { return InternalFlags; }
	void SetType(EntityType type) { Type = type; }
	EntityType GetType() { return Type; }
	void WriteNode(VMMDLL_SCATTER_HANDLE handle, int colour, bool show);
	void UpdatePosition(VMMDLL_SCATTER_HANDLE handle);
	void UpdateNode(VMMDLL_SCATTER_HANDLE handle);
	void UpdateHealth(VMMDLL_SCATTER_HANDLE handle);
	void UpdateBones();
	void UpdateHeadPosition(VMMDLL_SCATTER_HANDLE handle); // fast scatter read of all bone positions
	Vector3 GetBonePosition(int idx) const { return (idx >= 0 && idx < MAX_BONES) ? BonePositions[idx] : Vector3::Zero(); }
	void UpdateVelocity(); // Compute velocity from position delta

	// Velocity tracking for prediction (public for aimbot access)
	Vector3 Velocity;
	Vector3 PreviousPosition;
	std::chrono::steady_clock::time_point LastPositionTime;
	std::chrono::steady_clock::time_point LastMoveTime;
	bool HasPreviousPosition = false;

	void UpdateExtraction(VMMDLL_SCATTER_HANDLE handle);
	void UpdateClass(VMMDLL_SCATTER_HANDLE handle);
	uint64_t GetClass() { return ClassAddress; }
	bool IsHidden() { return Hidden; }
	void SetHidden(bool isHidden) { Hidden = isHidden; }
	bool GetValid() { return Valid; }
	void SetValid(bool valid) { Valid = valid; }
	std::string GetTypeAsString() { return Names[Type]; };
	bool IsHpChainValid() const {
		return HpPointer5 > 0x10000 && HpPointer5 < 0x7FFFFFFFFFFF;
	}
	void RetryHpChain();
	bool IsLocalPlayer();
	Vector3 GetPosition() const { return Position; }
	void SetWeapons(const std::string& w1, const std::string& w2) {
		WeaponName1 = w1;
		WeaponName2 = w2;
	}
	Vector3 GetHeadPosition() const { return HeadBonePtr != 0 ? BonePositions[0] : HeadPosition; }
	uint32_t GetBoneCount() const { return BoneCount; }
	const std::string& GetDebugBoneNames() const { return DebugBoneNames; }
	const Matrix4x4& GetWorldMatrix() const { return WorldMatrix; }
	void ApplyBoneWorldTransform();  // call after ExecuteReadScatter to convert local→world

	// ── Double-buffer render snapshot ──────────────────────────────────────
	EntityRenderData Render;       // front buffer — render thread reads this
	void CommitRenderData();        // copy live → Render (called under brief mutex)

	uint64_t SpecCountOffset1 = 0x198;
	uint64_t SpecCountOffset2 = 0x20;
	uint64_t SpecCountOffset3 = 0xD0;
	uint64_t SpecCountOffset4 = 0xE8;
	uint64_t SpecCountOffset5 = 0x330;
	uint64_t SpecCountPointer1 = 0x0;
	uint64_t SpecCountPointer2 = 0x0;
	uint64_t SpecCountPointer3 = 0x0;
	uint64_t SpecCountPointer4 = 0x0;
	int SpecCount = 0;

	static const uint32_t HIDDEN_FLAG = 0x8;
public:
	bool IsValid() const { return Valid; }
};