#pragma once

#include <string_view>

#include <RE/F/FormTypes.h>

constexpr const char* toFullString(const RE::FormType type) noexcept
{
    switch (type) {
    case RE::FormType::None:
        return "NONE (TESForm)";
    case RE::FormType::PluginInfo:
        return "TES4";
    case RE::FormType::FormGroup:
        return "GRUP";
    case RE::FormType::GameSetting:
        return "GMST";
    case RE::FormType::Keyword:
        return "KYWD (BGSKeyword)";
    case RE::FormType::LocationRefType:
        return "LCRT (BGSLocationRefType)";
    case RE::FormType::Action:
        return "AACT (BGSAction)";
    case RE::FormType::TextureSet:
        return "TXST (BGSTextureSet)";
    case RE::FormType::MenuIcon:
        return "MICN (BGSMenuIcon)";
    case RE::FormType::Global:
        return "GLOB (TESGlobal)";
    case RE::FormType::AcousticSpace:
        return "ASPC (BGSAcousticSpace)";
    case RE::FormType::Skill:
        return "SKIL";
    case RE::FormType::MagicEffect:
        return "MGEF (EffectSetting)";
    case RE::FormType::Script:
        return "SCPT (Script)";
    case RE::FormType::LandTexture:
        return "LTEX (TESLandTexture)";
    case RE::FormType::Enchantment:
        return "ENCH (EnchantmentItem)";
    case RE::FormType::Spell:
        return "SPEL (SpellItem)";
    case RE::FormType::Scroll:
        return "SCRL (ScrollItem)";
    case RE::FormType::Activator:
        return "ACTI (TESObjectACTI)";
    case RE::FormType::TalkingActivator:
        return "TACT (BGSTalkingActivator)";
    case RE::FormType::Misc:
        return "MISC (TESObjectMISC)";
    case RE::FormType::Apparatus:
        return "APPA (BGSApparatus)";
    case RE::FormType::Static:
        return "STAT (TESObjectSTAT)";
    case RE::FormType::StaticCollection:
        return "SCOL (BGSStaticCollection)";
    case RE::FormType::MovableStatic:
        return "MSTT (BGSMovableStatic)";
    case RE::FormType::Grass:
        return "GRAS (TESGrass)";
    case RE::FormType::Tree:
        return "TREE (TESObjectTREE)";
    case RE::FormType::Flora:
        return "FLOR (TESFlora)";
    case RE::FormType::Furniture:
        return "FURN (TESFurniture)";
    case RE::FormType::Weapon:
        return "WEAP (TESObjectWEAP)";
    case RE::FormType::Note:
        return "NOTE (BGSNote)";
    case RE::FormType::ConstructibleObject:
        return "COBJ (BGSConstructibleObject)";
    case RE::FormType::Projectile:
        return "PROJ (BGSProjectile)";
    case RE::FormType::Hazard:
        return "HAZD (BGSHazard)";
    case RE::FormType::SoulGem:
        return "SLGM (TESSoulGem)";
    case RE::FormType::LeveledItem:
        return "LVLI (TESLevItem)";
    case RE::FormType::Weather:
        return "WTHR (TESWeather)";
    case RE::FormType::Climate:
        return "CLMT (TESClimate)";
    case RE::FormType::ShaderParticleGeometryData:
        return "SPGD (BGSShaderParticleGeometryData)";
    case RE::FormType::ReferenceEffect:
        return "RFCT (BGSReferenceEffect)";
    case RE::FormType::ProjectileArrow:
        return "PARW (ArrowProjectile)";
    case RE::FormType::ProjectileGrenade:
        return "PGRE (GrenadeProjectile)";
    case RE::FormType::ProjectileBeam:
        return "PBEA (BeamProjectile)";
    case RE::FormType::ProjectileFlame:
        return "PFLA (FlameProjectile)";
    case RE::FormType::ProjectileCone:
        return "PCON (ConeProjectile)";
    case RE::FormType::ProjectileBarrier:
        return "PBAR (BarrierProjectile)";
    case RE::FormType::PlacedHazard:
        return "PHZD (Hazard)";
    case RE::FormType::WorldSpace:
        return "WRLD (TESWorldSpace)";
    case RE::FormType::Land:
        return "LAND (TESObjectLAND)";
    case RE::FormType::NavMesh:
        return "NAVM (NavMesh)";
    case RE::FormType::CombatStyle:
        return "CSTY (TESCombatStyle)";
    case RE::FormType::LoadScreen:
        return "LSCR (TESLoadScreen)";
    case RE::FormType::LeveledSpell:
        return "LVSP (TESLevSpell)";
    case RE::FormType::AnimatedObject:
        return "ANIO (TESObjectANIO)";
    case RE::FormType::Water:
        return "WATR (TESWaterForm)";
    case RE::FormType::EffectShader:
        return "EFSH (TESEffectShader)";
    case RE::FormType::TOFT:
        return "TOFT";
    case RE::FormType::Explosion:
        return "EXPL (BGSExplosion)";
    case RE::FormType::Debris:
        return "DEBR (BGSDebris)";
    case RE::FormType::ImageSpace:
        return "IMGS (TESImageSpace)";
    case RE::FormType::CameraShot:
        return "CAMS (BGSCameraShot)";
    case RE::FormType::CameraPath:
        return "CPTH (BGSCameraPath)";
    case RE::FormType::VoiceType:
        return "VTYP (BGSVoiceType)";
    case RE::FormType::MaterialType:
        return "MATT (BGSMaterialType)";
    case RE::FormType::Impact:
        return "IPCT (BGSImpactData)";
    case RE::FormType::ImpactDataSet:
        return "IPDS (BGSImpactDataSet)";
    case RE::FormType::Armature:
        return "ARMA (TESObjectARMA)";
    case RE::FormType::EncounterZone:
        return "ECZN (BGSEncounterZone)";
    case RE::FormType::Location:
        return "LCTN (BGSLocation)";
    case RE::FormType::Message:
        return "MESG (BGSMessage)";
    case RE::FormType::StoryManagerBranchNode:
        return "SMBN (BGSStoryManagerBranchNode)";
    case RE::FormType::StoryManagerQuestNode:
        return "SMQN (BGSStoryManagerQuestNode)";
    case RE::FormType::StoryManagerEventNode:
        return "SMEN (BGSStoryManagerEventNode)";
    case RE::FormType::DialogueBranch:
        return "DLBR (BGSDialogueBranch)";
    case RE::FormType::MusicTrack:
        return "MUST (BGSMusicTrackFormWrapper)";
    case RE::FormType::DialogueView:
        return "DLVW";
    case RE::FormType::WordOfPower:
        return "WOOP (TESWordOfPower)";
    case RE::FormType::Shout:
        return "SHOU (TESShout)";
    case RE::FormType::EquipSlot:
        return "EQUP (BGSEquipSlot)";
    case RE::FormType::Relationship:
        return "RELA (BGSRelationship)";
    case RE::FormType::SoundRecord:
        return "SNDR (BGSSoundDescriptorForm)";
    case RE::FormType::DualCastData:
        return "DUAL (BGSDualCastData)";
    case RE::FormType::SoundCategory:
        return "SNCT (BGSSoundCategory)";
    case RE::FormType::SoundOutputModel:
        return "SOPM (BGSSoundOutput)";
    case RE::FormType::CollisionLayer:
        return "COLL (BGSCollisionLayer)";
    case RE::FormType::ColorForm:
        return "CLFM (BGSColorForm)";
    case RE::FormType::ReverbParam:
        return "REVB (BGSReverbParameters)";
    case RE::FormType::LensFlare:
        return "LENS (BGSLensFlare)";
    case RE::FormType::LensSprite:
        return "LSPR";
    case RE::FormType::VolumetricLighting:
        return "VOLI (BGSVolumetricLighting)";
    case RE::FormType::Max:
        return "<RE::FormType::Max>";
    }

    return "<invalid_form_type>";
}

constexpr const char* toString(const RE::FormType type) noexcept
{
    switch (type) {
    case RE::FormType::None:
        return "NONE";
    case RE::FormType::PluginInfo:
        return "TES4";
    case RE::FormType::FormGroup:
        return "GRUP";
    case RE::FormType::GameSetting:
        return "GMST";
    case RE::FormType::Keyword:
        return "KYWD";
    case RE::FormType::LocationRefType:
        return "LCRT";
    case RE::FormType::Action:
        return "AACT";
    case RE::FormType::TextureSet:
        return "TXST";
    case RE::FormType::MenuIcon:
        return "MICN";
    case RE::FormType::Global:
        return "GLOB";
    case RE::FormType::AcousticSpace:
        return "ASPC";
    case RE::FormType::Skill:
        return "SKIL";
    case RE::FormType::MagicEffect:
        return "MGEF";
    case RE::FormType::Script:
        return "SCPT";
    case RE::FormType::LandTexture:
        return "LTEX";
    case RE::FormType::Enchantment:
        return "ENCH";
    case RE::FormType::Spell:
        return "SPEL";
    case RE::FormType::Scroll:
        return "SCRL";
    case RE::FormType::Activator:
        return "ACTI";
    case RE::FormType::TalkingActivator:
        return "TACT";
    case RE::FormType::Misc:
        return "MISC";
    case RE::FormType::Apparatus:
        return "APPA";
    case RE::FormType::Static:
        return "STAT";
    case RE::FormType::StaticCollection:
        return "SCOL";
    case RE::FormType::MovableStatic:
        return "MSTT";
    case RE::FormType::Grass:
        return "GRAS";
    case RE::FormType::Tree:
        return "TREE";
    case RE::FormType::Flora:
        return "FLOR";
    case RE::FormType::Furniture:
        return "FURN";
    case RE::FormType::Weapon:
        return "WEAP";
    case RE::FormType::Note:
        return "NOTE";
    case RE::FormType::ConstructibleObject:
        return "COBJ";
    case RE::FormType::Projectile:
        return "PROJ";
    case RE::FormType::Hazard:
        return "HAZD";
    case RE::FormType::SoulGem:
        return "SLGM";
    case RE::FormType::LeveledItem:
        return "LVLI";
    case RE::FormType::Weather:
        return "WTHR";
    case RE::FormType::Climate:
        return "CLMT";
    case RE::FormType::ShaderParticleGeometryData:
        return "SPGD";
    case RE::FormType::ReferenceEffect:
        return "RFCT";
    case RE::FormType::ProjectileArrow:
        return "PARW";
    case RE::FormType::ProjectileGrenade:
        return "PGRE";
    case RE::FormType::ProjectileBeam:
        return "PBEA";
    case RE::FormType::ProjectileFlame:
        return "PFLA";
    case RE::FormType::ProjectileCone:
        return "PCON";
    case RE::FormType::ProjectileBarrier:
        return "PBAR";
    case RE::FormType::PlacedHazard:
        return "PHZD";
    case RE::FormType::WorldSpace:
        return "WRLD";
    case RE::FormType::Land:
        return "LAND";
    case RE::FormType::NavMesh:
        return "NAVM";
    case RE::FormType::CombatStyle:
        return "CSTY";
    case RE::FormType::LoadScreen:
        return "LSCR";
    case RE::FormType::LeveledSpell:
        return "LVSP";
    case RE::FormType::AnimatedObject:
        return "ANIO";
    case RE::FormType::Water:
        return "WATR";
    case RE::FormType::EffectShader:
        return "EFSH";
    case RE::FormType::TOFT:
        return "TOFT";
    case RE::FormType::Explosion:
        return "EXPL";
    case RE::FormType::Debris:
        return "DEBR";
    case RE::FormType::ImageSpace:
        return "IMGS";
    case RE::FormType::CameraShot:
        return "CAMS";
    case RE::FormType::CameraPath:
        return "CPTH";
    case RE::FormType::VoiceType:
        return "VTYP";
    case RE::FormType::MaterialType:
        return "MATT";
    case RE::FormType::Impact:
        return "IPCT";
    case RE::FormType::ImpactDataSet:
        return "IPDS";
    case RE::FormType::Armature:
        return "ARMA";
    case RE::FormType::EncounterZone:
        return "ECZN";
    case RE::FormType::Location:
        return "LCTN";
    case RE::FormType::Message:
        return "MESG";
    case RE::FormType::StoryManagerBranchNode:
        return "SMBN";
    case RE::FormType::StoryManagerQuestNode:
        return "SMQN";
    case RE::FormType::StoryManagerEventNode:
        return "SMEN";
    case RE::FormType::DialogueBranch:
        return "DLBR";
    case RE::FormType::MusicTrack:
        return "MUST";
    case RE::FormType::DialogueView:
        return "DLVW";
    case RE::FormType::WordOfPower:
        return "WOOP";
    case RE::FormType::Shout:
        return "SHOU";
    case RE::FormType::EquipSlot:
        return "EQUP";
    case RE::FormType::Relationship:
        return "RELA";
    case RE::FormType::SoundRecord:
        return "SNDR";
    case RE::FormType::DualCastData:
        return "DUAL";
    case RE::FormType::SoundCategory:
        return "SNCT";
    case RE::FormType::SoundOutputModel:
        return "SOPM";
    case RE::FormType::CollisionLayer:
        return "COLL";
    case RE::FormType::ColorForm:
        return "CLFM";
    case RE::FormType::ReverbParam:
        return "REVB";
    case RE::FormType::LensFlare:
        return "LENS";
    case RE::FormType::LensSprite:
        return "LSPR";
    case RE::FormType::VolumetricLighting:
        return "VOLI";
    case RE::FormType::Max:
        return "<RE::FormType::Max>";
    }

    return "<invalid_form_type>";
}
