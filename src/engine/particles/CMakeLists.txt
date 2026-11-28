# Sources of this folder (see vgears_sources in src/CMakeLists.txt).
vgears_sources(
    Particle.cpp
    ParticleEmitter.cpp
    ParticleEmitterDictionary.cpp
    ParticleEmitterTranslator.cpp
    ParticleRenderer.cpp
    ParticleRendererTranslator.cpp
    ParticleSystem.cpp
    ParticleSystemFactory.cpp
    ParticleSystemManager.cpp
    ParticleSystemTranslator.cpp
    ParticleSystemTranslatorManager.cpp
    ParticleTechnique.cpp
    ParticleTechniqueTranslator.cpp
    ParticleVisual.cpp
)

add_subdirectory(emitters)
add_subdirectory(renderer)
