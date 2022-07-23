#include "OgreBase.h"
#include <iostream>
#include <OgreFontManager.h>
#include <OgreOverlay.h>
//#include <OgreOverlayManager.h>
#include <Overlay/OgreOverlayManager.h>

Ogre::Root*                     root;
Ogre::RenderWindow*             window;
std::vector< Ogre::Entity* >    entitys;
Ogre::Camera                   *camera;
DisplayFrameListener           *frame_listener;

void
InitializeOgreBase( const Ogre::String& name )
{
    std::cout << "[InitializeOgreBase] START" << std::endl;
    Ogre::LogManager* log_manager = new Ogre::LogManager();
    log_manager->createLog( "q-gears.log", true, true );
    log_manager->getDefaultLog()->setLogDetail( ( Ogre::LoggingLevel )3 );

    Ogre::String ressource_cfg("");
    Ogre::String plugins_cfg("");
    Ogre::String dyn_lib_ext("");
    Ogre::String render_system("");
    std::cout << "[InitializeOgreBase] 1" << std::endl;

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    dyn_lib_ext = ".dll";
    std::cout << "[InitializeOgreBase] 1 LIBW32" << std::endl;
#else // Assume Linux for now
    dyn_lib_ext = ".so";
    std::cout << "[InitializeOgreBase] 1 LIBLINUX" << std::endl;
#endif

#ifdef NDEBUG
    std::cout << "[InitializeOgreBase] 1 NO DEBUG" << std::endl;
    ressource_cfg = "resources.cfg";
    plugins_cfg = "plugins.cfg";
    render_system = "./RenderSystem_GL" + dyn_lib_ext;
#else
    std::cout << "[InitializeOgreBase] 1 DEBUG" << std::endl;
    ressource_cfg = "resources_d.cfg";
    plugins_cfg = "plugins_d.cfg";
    render_system = "./RenderSystem_GL_d" + dyn_lib_ext;
#endif

    // init root early
    root = new Ogre::Root( plugins_cfg );
    std::cout << "[InitializeOgreBase] 2" << std::endl;

    // set up resources
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load( ressource_cfg );

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
    std::cout << "[InitializeOgreBase] 3" << std::endl;
    Ogre::String secName, typeName, archName;
    Ogre::ResourceGroupManager &res_gm( Ogre::ResourceGroupManager::getSingleton() );
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            res_gm.addResourceLocation( archName, typeName, secName, true );
        }
    }
    std::cout << "[InitializeOgreBase] 4" << std::endl;

    // configure
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    // IVV ERROR Compili\ng
    //if( !root->restoreConfig() && !root->showConfigDialog() )
    //{
        root->setRenderSystem( root->getAvailableRenderers()[ 0 ] );
    //}*/

    root->initialise( false );
    std::cout << "[InitializeOgreBase] 5" << std::endl;
    Ogre::NameValuePairList misc;
    misc[ "title" ] = name;
    window = root->createRenderWindow( "QGearsWindow", 800, 600, false, &misc );


    // initialize resource
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation( "./", "FileSystem", "General" );
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation( "./exported", "FileSystem", "General" );
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

    Ogre::SceneManager* scene_manager;
    Ogre::Viewport*     viewport;

    Ogre::FontManager* fmgr = new Ogre::FontManager;
    Ogre::OverlayManager* overlay = new Ogre::OverlayManager();

    frame_listener = new DisplayFrameListener( window );
    root->addFrameListener( frame_listener );

    scene_manager = root->createSceneManager( Ogre::ST_GENERIC, "Scene" );
    scene_manager->clearScene();
    scene_manager->setAmbientLight( Ogre::ColourValue( 0.5, 0.5, 0.5 ) );
    Ogre::Light* directionalLight = scene_manager->createLight("directionalLight");
    directionalLight->setType( Ogre::Light::LT_DIRECTIONAL );
    directionalLight->setDiffuseColour( Ogre::ColourValue( 0.5, 0.5, 0.5) );
    directionalLight->setSpecularColour( Ogre::ColourValue( 0.5, 0.5, 0.5) );
    directionalLight->setDirection( Ogre::Vector3( 0, 0, -1 ) );

    camera = scene_manager->createCamera( "Camera" );
    camera->setNearClipDistance( 0.01f );
    camera->setPosition( 0, 5, 10 );
    camera->lookAt( 0, 0, 0 );

    viewport = window->addViewport( camera );
    viewport->setBackgroundColour( Ogre::ColourValue( 0.0f, 0.4f, 0.0f ) );
    camera->setAspectRatio( Ogre::Real( viewport->getActualWidth() ) / Ogre::Real( viewport->getActualHeight() ) );

    LOGGER = new Logger( "game.log" );
}



void
DeinitializeOgreBase()
{
    delete LOGGER;

    delete root;
    delete frame_listener;
}
