#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/qtime/QuickTimeGl.h"
#include "cinder/params/Params.h"
#include "cinder/ConcurrentCircularBuffer.h"
#include "cinder/Timer.h"
#include "cinder/Utilities.h"
#include "signals/SignalsManager.h"
#include "statusbar/status.h"



using namespace ci;
using namespace ci::app;
using namespace std;

class NormalGetterApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
  private:
    ~NormalGetterApp();
    void loadMovie();
    void setupParams();
    void loadGlsl();
    void updateFbos();
    void processBatch();
    void normalize(gl::TextureRef _tex);
    ci::ConcurrentCircularBuffer<std::pair<ImageSourceRef,int>>	*mPreprocessedImages;
    params::InterfaceGlRef mParams;
    ci::qtime::MovieGlRef  mMovie;
    ci::gl::GlslProgRef     mNormalGlsl;
    StatusRef mStatus;
    float  bias  = 50.f;
    bool invertR, invertG = false;
    gl::TextureRef mTex;
    fs::path mMoviePath;
    vec2 movieSize;
    gl::FboRef mFbo, mOutputFbo;
    bool makeMovie, pushFramesToBuffer, mPolling = false;
    int currentFrame, totalFrames = 0;
    std::shared_ptr<std::thread> mThread;
    fs::path saveDirectory;
    std::string directory;
};

NormalGetterApp::~NormalGetterApp(){
    mPolling = false;
    if (mThread->joinable()) mThread->join();
}


void NormalGetterApp::setup()
{
    setWindowSize(640,480);
    loadGlsl();
    setupParams();
    mPolling = true;
    mStatus = Status::create();
    saveDirectory = getHomeDirectory();
    directory = saveDirectory.string();
    pushFramesToBuffer = false;
    makeMovie = false;

}

void NormalGetterApp::setupParams(){
    mParams = params::InterfaceGl::create( "Normal Getter", vec2( 300, 220 ) );
    mParams->addSeparator();
    mParams->addButton("InvertG", [&](){
        invertG = !invertG;
    });
    mParams->addButton("InvertR", [&](){
        invertR =! invertR;
    });
    mParams->addParam("bias", &bias).min(0.f).max(100.f);
    mParams->addButton( "Load movie", [ & ](){
        makeMovie = true;
    } );
    mParams->addButton( "Play movie", [ & ](){
        if(mMovie) mMovie->play();
    } );
    mParams->addButton( "Stop movie", [ & ](){
        if(mMovie) mMovie->stop();
    } );
    mParams->addButton( "Choose Save Directory", [ & ](){
        saveDirectory = getFolderPath();
        directory = saveDirectory.string();
    } );
    mParams->addParam("Save Directory", &directory).updateFn([&](){
        saveDirectory = directory;
    });
    mParams->addButton( "Process Frame", [ & ](){
        if(mMovie && mOutputFbo)writeImage(saveDirectory / string(to_string(1000000 +int(mMovie->getCurrentTime()*mMovie->getFramerate()))+ ".png"), mOutputFbo->getColorTexture()->createSource(), ImageTarget::Options(),"png");
    } );
    mParams->addButton( "Process Batch", [ & ](){
        if(!pushFramesToBuffer && mMovie){
            pushFramesToBuffer = true;
            currentFrame = 0;
            mMovie->seekToStart();
            mMovie->setLoop(false);
            mMovie->play();
        }
    } );

}
void NormalGetterApp::processBatch(){
    ThreadSetup threadSetup;
    while(mPolling){
        ci::sleep(50);
        if(mPreprocessedImages->isNotEmpty() ) {
            std::pair<ci::ImageSourceRef, int> mNewImageSource;
            mPreprocessedImages->popBack( &mNewImageSource );
            SignalsManager::Instance().status.emit(float(mNewImageSource.second)/float(totalFrames),true);
            try{
                writeImage(saveDirectory / string(to_string(1000000 + mNewImageSource.second) + ".png") , mNewImageSource.first, ImageTarget::Options(),"png");
            }catch (...){
                
            }
        }
    }
}

void NormalGetterApp::mouseDown( MouseEvent event )
{

}

void NormalGetterApp::loadGlsl(){
    
    try {
        mNormalGlsl = gl::GlslProg::create( ci::app::loadAsset( "normal.vert" ), ci::app::loadAsset( "normal.frag" ) );
    
    }
    catch (const ci::gl::GlslProgCompileExc &ex){
        std::cout << "Normal GLSL Error: " << ex.what() << std::endl;
    }
    catch (gl::GlslNullProgramExc ex) {
        std::cout << "Normal GLSL Error: " << ex.what() << std::endl;
    }
    

}

void NormalGetterApp::loadMovie(){
    try{
        mMoviePath = ci::app::getOpenFilePath();
        mMovie = qtime::MovieGl::create( mMoviePath );
        currentFrame = 0;
        mPreprocessedImages = new ci::ConcurrentCircularBuffer<std::pair<ci::ImageSourceRef,int>>(mMovie->getNumFrames());
        mThread = shared_ptr<thread>( new thread( bind( &NormalGetterApp::processBatch, this ) ));
        
    }catch (...){
        makeMovie = false;
        return;
    }
    setFrameRate(mMovie->getFramerate());
    mMovie->play();
    mMovie->setLoop();
    movieSize = mMovie->getSize();
    totalFrames = mMovie->getNumFrames();
    getWindow()->setSize(movieSize);
    makeMovie = false;

}

void NormalGetterApp::normalize(gl::TextureRef _tex){
    {
        gl::ScopedMatrices push;
        gl::ScopedFramebuffer fbo(mOutputFbo);
        gl::clear();
        ci::gl::setMatricesWindow( mOutputFbo->getSize() );
        ci::gl::ScopedViewport view( ci::vec2(0), mOutputFbo->getSize() );
        gl::ScopedGlslProg mGlsl(mNormalGlsl);
        gl::ScopedTextureBind tex0(_tex);
        mNormalGlsl->uniform("uSampler", 0);
        mNormalGlsl->uniform("u_textureSize", vec2(_tex->getWidth(), _tex->getHeight()));
        mNormalGlsl->uniform("bias", bias);
        mNormalGlsl->uniform("invertR", float(invertR ? -1.0 : 1.0) );
        mNormalGlsl->uniform("invertG", float(invertG ? -1.0 : 1.0));
        gl::drawSolidRect(Rectf(vec2(0), _tex->getSize()));
    }
    if( pushFramesToBuffer){
        mPreprocessedImages->pushFront(std::make_pair(mOutputFbo->getColorTexture()->createSource(), currentFrame));
        if(currentFrame == mMovie->getNumFrames()){
            pushFramesToBuffer = false;
            mMovie->setLoop(true);
            mMovie->seekToStart();
        }
        currentFrame++;
    }
}

void NormalGetterApp::updateFbos(){
    if( mMovie && !makeMovie){
        if(!mFbo) mFbo = gl::Fbo::create(movieSize.x, movieSize.y);
        if( !mOutputFbo) mOutputFbo = gl::Fbo::create(movieSize.x, movieSize.y);
        
        if(mFbo){
            gl::ScopedMatrices push;
            gl::ScopedFramebuffer fbo(mFbo);
            ci::gl::clear(ci::Color(0,0,0));
            ci::gl::setMatricesWindow( mFbo->getSize() );
            ci::gl::ScopedViewport view( ci::vec2(0), mFbo->getSize() );
            gl::draw(mMovie->getTexture());
        }
        
    }

}
                   
void NormalGetterApp::update()
{
    if(makeMovie ){
        loadMovie();
    }
    
    updateFbos();
    
}

void NormalGetterApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
    if(mFbo && mOutputFbo){
        normalize(mFbo->getColorTexture());
        gl::draw(mOutputFbo->getColorTexture());
    }
    
  //  if(mFbo) gl::draw(mFbo->getColorTexture());
    mStatus->draw();
    mParams->draw();
}

CINDER_APP( NormalGetterApp, RendererGl )
