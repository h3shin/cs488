#include "viewer.hpp"
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <algorithm>

#define TO_PI (3.14159265 / 180.0)
#define SCALE_MAX 1.25
#define SCALE_MIN 0.5

Viewer::Viewer()
{
  Glib::RefPtr<Gdk::GL::Config> glconfig;

  // Ask for an OpenGL Setup with
  //  - red, green and blue component colour
  //  - a depth buffer to avoid things overlapping wrongly
  //  - double-buffered rendering to avoid tearing/flickering
  glconfig = Gdk::GL::Config::create(Gdk::GL::MODE_RGB |
                                     Gdk::GL::MODE_DEPTH |
                                     Gdk::GL::MODE_DOUBLE);
  if (glconfig == 0) {
    // If we can't get this configuration, die
    std::cerr << "Unable to setup OpenGL Configuration!" << std::endl;
    abort();
  }

  // Accept the configuration
  set_gl_capability(glconfig);

  // Register the fact that we want to receive these events
  add_events(Gdk::BUTTON1_MOTION_MASK    |
             Gdk::BUTTON2_MOTION_MASK    |
             Gdk::BUTTON3_MOTION_MASK    |
             Gdk::BUTTON_PRESS_MASK      | 
             Gdk::BUTTON_RELEASE_MASK    |
             Gdk::VISIBILITY_NOTIFY_MASK);

  m_game = new Game(10,20);

  m_color_mode = WIRE_FRAME;

  m_angle[0] = m_angle[1] = m_angle[2] =
  m_button_press_angle[1] = m_button_press_angle[2] = m_button_press_angle[3] = 0.0;
  m_button_number[1] = m_button_number[2] = m_button_number[3] = false;
  m_shift = false;
  m_scale = 1.0;
  m_doublebuffer = 1;
  m_disconnect = false;
  m_gameover = false;

  m_speed[SLOW] = 500, m_speed[MEDIUM] = 300, m_speed[FAST] = 150;
  m_speed_mode = SLOW;
  sigc::slot<bool> tickslot = sigc::mem_fun(*this, &Viewer::timeout_handler);
  Glib::signal_timeout().connect(tickslot, m_speed[m_speed_mode]);
  sigc::slot<bool> renderslot = sigc::mem_fun(*this, &Viewer::render_handler);
  Glib::signal_timeout().connect(renderslot, 10);

/*
  m_color[0][0] = 1.0, m_color[0][1] = 0.0, m_color[0][2] = 0.0; //red
  m_color[1][0] = 0.0, m_color[1][1] = 1.0, m_color[1][2] = 0.0; //green
  m_color[2][0] = 0.0, m_color[2][1] = 0.0, m_color[2][2] = 1.0; //blue
  m_color[3][0] = 1.0, m_color[3][1] = 0.2, m_color[3][2] = 0.0; //orange
  m_color[4][0] = 1.0, m_color[4][1] = 0.0, m_color[4][2] = 0.4; //pink
  m_color[5][0] = 0.4, m_color[5][1] = 0.0, m_color[5][2] = 0.4; //purple
  m_color[6][0] = 1.0, m_color[6][1] = 1.0, m_color[6][2] = 0.0; //yellow
  m_color[7][0] = 0.2, m_color[7][1] = 0.0, m_color[7][2] = 0.0; //brown
*/
  m_color[0][0] = 0.941, m_color[0][1] = 0.502, m_color[0][2] = 0.502; //light coral
  m_color[1][0] = 1.0, m_color[1][1] = 0.412, m_color[1][2] = 0.706; //hot pink
  m_color[2][0] = 0.863, m_color[2][1] = 0.078, m_color[2][2] = 0.235; //crimson
  m_color[3][0] = 1.0, m_color[3][1] = 0.647, m_color[3][2] = 0.0; //orange
  m_color[4][0] = 0.42, m_color[4][1] = 0.557, m_color[4][2] = 0.137; //olive drap
  m_color[5][0] = 0.529, m_color[5][1] = 0.808, m_color[5][2] = 0.980; //light skyblue
  m_color[6][0] = 0.118, m_color[6][1] = 0.565, m_color[6][2] = 1.0; //dodgerblue
  m_color[7][0] = 0.627, m_color[7][1] = 0.322, m_color[7][2] = 0.176; //sienna
}

Viewer::~Viewer()
{
  // Nothing to do here right now.
  delete m_game;
}

bool Viewer::timeout_handler()
{
//  std::cerr << "tick" << std::endl;
  if ( m_game->tick() == -1 || m_disconnect )
  {
    m_gameover = true;
    std::cerr << "timeout_handler terminated" << std::endl;
    return false;
  }
  if ( m_disconnect )
  {
    m_disconnect = false;
    sigc::slot<bool> tslot = sigc::mem_fun(*this, &Viewer::timeout_handler);
    Glib::signal_timeout().connect(tslot, m_speed[m_speed_mode]);
    invalidate();
  }
  render_image(false, 0.0);
  return true;
}

bool Viewer::render_handler()
{
  render_image(false, 0.0);
  return !m_gameover;
}
void Viewer::set_speed_mode(SpeedMode mode)
{
  m_disconnect = true;
  m_speed_mode = mode;
//  std::cerr << "speed mode: " << m_speed_mode << ", speed: " << m_speed[m_speed_mode] << std::endl;
}

Game* Viewer::get_game()
{
  return m_game;
}

void Viewer::invalidate()
{
  //Force a rerender
  Gtk::Allocation allocation = get_allocation();
  get_window()->invalidate_rect( allocation, false);
}

void Viewer::on_realize()
{
  // Do some OpenGL setup.
  // First, let the base class do whatever it needs to
  Gtk::GL::DrawingArea::on_realize();
  
  Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();
  
  if (!gldrawable)
    return;

  if (!gldrawable->gl_begin(get_gl_context()))
    return;

  // Just enable depth testing and set the background colour.
  glEnable(GL_DEPTH_TEST);
//  glClearColor(0.7, 0.7, 1.0, 0.0);
  glClearColor(1.0, 1.0, 1.0, 0.0);
  gldrawable->gl_end();
}


void Viewer::drawVertex(int posn, float x, float y, float z)
{
  switch(posn)
  {
    case 1:
      glVertex3f(0.0f+x, 1.0f+y, 1.0f+z);
    break;
    case 2:
      glVertex3f(1.0f+x, 1.0f+y, 1.0f+z);
    break;
    case 3:
      glVertex3f(0.0f+x, 0.0f+y, 1.0f+z);
    break;
    case 4:
      glVertex3f(1.0f+x, 0.0f+y, 1.0f+z);
    break;
    case 5:
      glVertex3f(0.0f+x, 1.0f+y, 0.0f+z);
    break;
    case 6:
      glVertex3f(1.0f+x, 1.0f+y, 0.0f+z);
    break;
    case 7:
      glVertex3f(0.0f+x, 0.0f+y, 0.0f+z);
    break;
    default: //case 8
      glVertex3f(1.0f+x, 0.0f+y, 0.0f+z);
  }
}

void Viewer::drawCube(float x, float y, float z, int cindex)
{
    glBegin(GL_LINES);
    glNormal3f(0.0f, 0.0f, 0.0f);

    drawVertex(1, x, y, z); drawVertex(3, x, y, z);
    drawVertex(2, x, y, z); drawVertex(4, x, y, z);
    drawVertex(6, x, y, z); drawVertex(8, x, y, z);
    drawVertex(5, x, y, z); drawVertex(7, x, y, z);

    //Front
    drawVertex(1, x, y, z); drawVertex(2, x, y, z);
    drawVertex(4, x, y, z); drawVertex(3, x, y, z);

    //Back
    drawVertex(5, x, y, z); drawVertex(6, x, y, z);
    drawVertex(8, x, y, z); drawVertex(7, x, y, z);

    //Left
    drawVertex(1, x, y, z); drawVertex(5, x, y, z);
    drawVertex(7, x, y, z); drawVertex(3, x, y, z);

    //Right
    drawVertex(6, x, y, z); drawVertex(2, x, y, z);
    drawVertex(4, x, y, z); drawVertex(8, x, y, z);

    //Bottom
    drawVertex(7, x, y, z); drawVertex(3, x, y, z);
    drawVertex(4, x, y, z); drawVertex(8, x, y, z);

    //Top
    drawVertex(5, x, y, z); drawVertex(1, x, y, z);
    drawVertex(2, x, y, z); drawVertex(6, x, y, z);
    glEnd();

    if ( m_color_mode != WIRE_FRAME )
    {
      if ( m_color_mode == FACE )
      {
        glColor3f(m_color[cindex][0], m_color[cindex][1], m_color[cindex][2]);
      }

      glBegin(GL_QUADS);

      std::vector<int> colorVector;
      std::vector<int>::iterator it;
      if ( m_color_mode == CHRISTMAS )
      {
        for(int i = 0; i < 7; ++i) colorVector.push_back(i);
        std::random_shuffle ( colorVector.begin(), colorVector.end() );
        it = colorVector.begin();
      }
      //Front
      if ( m_color_mode == CHRISTMAS ) glColor3f(m_color[*it][0], m_color[*it][1], m_color[*it++][2]);
      else if ( m_color_mode == MULTICOLOR )
      {
        int newindex = (cindex+1)%8;
        glColor3f(m_color[newindex][0], m_color[newindex][1], m_color[newindex][2]);
      }
      glNormal3f(0.0f, 0.0f, 1.0f);
      drawVertex(1, x, y, z); drawVertex(2, x, y, z);
      drawVertex(4, x, y, z); drawVertex(3, x, y, z);

      //Back
      if ( m_color_mode == CHRISTMAS ) glColor3f(m_color[*it][0], m_color[*it][1], m_color[*it++][2]);
      else if ( m_color_mode == MULTICOLOR )
      {
        int newindex = (cindex+2)%8;
        glColor3f(m_color[newindex][0], m_color[newindex][1], m_color[newindex][2]);
      }

      glNormal3f(0.0f, 0.0f, -1.0f);
      drawVertex(5, x, y, z); drawVertex(6, x, y, z);
      drawVertex(8, x, y, z); drawVertex(7, x, y, z);

      //Left
      if ( m_color_mode == CHRISTMAS ) glColor3f(m_color[*it][0], m_color[*it][1], m_color[*it++][2]);
      else if ( m_color_mode == MULTICOLOR )
      {
        int newindex = (cindex+3)%8;
        glColor3f(m_color[newindex][0], m_color[newindex][1], m_color[newindex][2]);
      }

      glNormal3f(-1.0f, 0.0f, 0.0f);
      drawVertex(1, x, y, z); drawVertex(5, x, y, z);
      drawVertex(7, x, y, z); drawVertex(3, x, y, z);

      //Right
      if ( m_color_mode == CHRISTMAS ) glColor3f(m_color[*it][0], m_color[*it][1], m_color[*it++][2]);
      else if ( m_color_mode == MULTICOLOR )
      {
        int newindex = (cindex+4)%8;
        glColor3f(m_color[newindex][0], m_color[newindex][1], m_color[newindex][2]);
      }
      glNormal3f(1.0f, 0.0f, -0.6f);
      drawVertex(6, x, y, z); drawVertex(2, x, y, z);
      drawVertex(4, x, y, z); drawVertex(8, x, y, z);

      //Bottom
      if ( m_color_mode == CHRISTMAS ) glColor3f(m_color[*it][0], m_color[*it][1], m_color[*it++][2]);
      else if ( m_color_mode == MULTICOLOR )
      {
        int newindex = (cindex+5)%8;
        glColor3f(m_color[newindex][0], m_color[newindex][1], m_color[newindex][2]);
      }
      glNormal3f(0.0f, 1.0f, 0.0f);
      drawVertex(7, x, y, z); drawVertex(3, x, y, z);
      drawVertex(4, x, y, z); drawVertex(8, x, y, z);

      //Top
      if ( m_color_mode == CHRISTMAS ) glColor3f(m_color[*it][0], m_color[*it][1], m_color[*it++][2]);
      else if ( m_color_mode == MULTICOLOR )
      {
        int newindex = (cindex+6)%8;
        glColor3f(m_color[newindex][0], m_color[newindex][1], m_color[newindex][2]);
      }
      glNormal3f(0.0f, -1.0f, 0.5f);
      drawVertex(5, x, y, z); drawVertex(1, x, y, z);
      drawVertex(2, x, y, z); drawVertex(6, x, y, z);
  }
  glEnd();
}

void Viewer::new_game()
{
  m_game->reset();
  invalidate();
}

void Viewer::reset_game()
{
  m_angle[0] = m_angle[1] = m_angle[2] =
  m_button_press_angle[1] = m_button_press_angle[2] = m_button_press_angle[3] = 0.0;
  m_button_number[1] = m_button_number[2] = m_button_number[3] = false;
  m_shift = false;
  m_scale = 1.0;
}

bool Viewer::on_expose_event(GdkEventExpose* event)
{
/*
  Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

  if (!gldrawable) return false;

  if (!gldrawable->gl_begin(get_gl_context()))
    return false;

  // Clear the screen
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Modify the current projection matrix so that we move the 
  // camera away from the origin.  We'll draw the game at the
  // origin, and we need to back up to see it.

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glTranslated(0.0, 0.0, -40.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Not implemented: set up lighting (if necessary)

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_NORMALIZE);

  //Add ambient light
  GLfloat ambientColor[] = {0.2f, 0.2f, 0.2f, 1.0f}; //Color(0.2, 0.2, 0.2)
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

  // Not implemented: scale and rotate the scene

  // You'll be drawing unit cubes, so the game will have width
  // 10 and height 24 (game = 20, stripe = 4).  Let's translate
  // the game so that we can draw it starting at (0,0) but have
  // it appear centered in the window.
  glTranslated(-5.0, -12.0, 0.0);

  // Not implemented: actually draw the current game state.
  // Here's some test code that draws red triangles at the
  // corners of the game board.

  glColor3d(1.0, 0.0, 0.0);
  glBegin(GL_TRIANGLES);
  glVertex3d(0.0, 0.0, 0.0);
  glVertex3d(1.0, 0.0, 0.0);
  glVertex3d(0.0, 1.0, 0.0);
  glVertex3d(9.0, 0.0, 0.0);
  glVertex3d(10.0, 0.0, 0.0);
  glVertex3d(10.0, 1.0, 0.0);
  glVertex3d(0.0, 19.0, 0.0);
  glVertex3d(1.0, 20.0, 0.0);
  glVertex3d(0.0, 20.0, 0.0);
  glVertex3d(10.0, 19.0, 0.0);
  glVertex3d(10.0, 20.0, 0.0);
  glVertex3d(9.0, 20.0, 0.0);
  glEnd();

  // We pushed a matrix onto the PROJECTION stack earlier, we 
  // need to pop it.

  // Draw wall
//  m_color_mode = FACE;
//  glRotatef(-20.0f, 0.0f, 1.0f, 0.0f);
//  drawCube(0.0,0.0,0.0);

//  glScalef(0.5f,0.5f,0.5f);

  glColor3f(0.4f, 0.0f, 0.0f);
  for ( int i = 0; i < 10; ++i )
  {
      drawCube((float)i,0.0,0.0);
  }
  for ( int i = 1; i < 20; ++i )
  {
      drawCube(0.0,(float)i,0.0);
      drawCube(9.0,(float)i,0.0);
  }

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  // Swap the contents of the front and back buffers so we see what we
  // just drew. This should only be done if double buffering is enabled.
  gldrawable->swap_buffers();

  gldrawable->gl_end();
*/
  return render_image(false, 0.0);
}

bool Viewer::on_configure_event(GdkEventConfigure* event)
{
  Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

  if (!gldrawable) return false;
  
  if (!gldrawable->gl_begin(get_gl_context()))
    return false;

  // Set up perspective projection, using current size and aspect
  // ratio of display

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, event->width, event->height);
  gluPerspective(40.0, (GLfloat)event->width/(GLfloat)event->height, 0.1, 1000.0);

  // Reset to modelview matrix mode
  
  glMatrixMode(GL_MODELVIEW);

  gldrawable->gl_end();

  return true;
}

bool Viewer::on_button_press_event(GdkEventButton* event)
{
  std::cerr << "Stub: Button " << event->button << " pressed" << std::endl;
  std::cerr << "Stub: x: " << event->x << " pressed" << std::endl;
  std::cerr << "Stub: y: " << event->y << " pressed" << std::endl;

  if ( m_button_number[event->button] )
  {
     m_button_number[event->button] = false;
  }
  else
  {
    m_button_number[event->button] = true;
    m_button_press_scale = m_button_press_angle[event->button] = event->x;
  }
  return true;
}

bool Viewer::on_button_release_event(GdkEventButton* event)
{
  std::cerr << "Stub: Button " << event->button << " released" << std::endl;
  
//  m_button_number[event->button] = false;
  return true;
}

bool Viewer::render_image(bool useData, float data)
{
  useData = m_useData;
  Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

  if (!gldrawable) return false;

  if (!gldrawable->gl_begin(get_gl_context()))
    return false;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  int buf = m_doublebuffer;
  std::cerr << buf << std::endl;
  if ( buf == -1 )
  {
    glDrawBuffer(GL_FRONT);
  }
  else
  {
    glDrawBuffer(GL_BACK);
  }

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glTranslated(0.0, 0.0, -40.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_NORMALIZE);

  //Add ambient light
  GLfloat ambientColor[] = {0.2f, 0.2f, 0.2f, 1.0f}; //Color(0.2, 0.2, 0.2)
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

 // Not implemented: scale and rotate the scene

  // You'll be drawing unit cubes, so the game will have width
  // 10 and height 24 (game = 20, stripe = 4).  Let's translate
  // the game so that we can draw it starting at (0,0) but have
  // it appear centered in the window.
  glTranslated(-5.0, -12.0, 0.0);

  rotate_image(useData, data);
  scale_image(useData, data);
  drawWall();
  drawPieces();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  // Swap the contents of the front and back buffers so we see what we
  // just drew. This should only be done if double buffering is enabled.
  if ( buf == 1 )
  {
     gldrawable->swap_buffers();
  }
  else // -1
  {
    glFlush();
  }

  gldrawable->gl_end();

  return true;
}

bool Viewer::on_motion_notify_event(GdkEventMotion* event)
{
  std::cerr << "Stub: Motion at " << event->x << ", " << event->y << std::endl;
  return render_image(true,event->x);
}

void Viewer::drawPieces()
{
  int width = m_game->getWidth();
  int height = m_game->getHeight()+4; //TODO: Make it member variable??
//  std::cerr << "w: " << width << ", h: " << height << std::endl;
  for ( int r = 0; r < height; ++r ) {
    for ( int c = 0; c < width; ++c ) {
       int cindex = m_game->get(r,c);
       if ( cindex != -1 )
       {
//         std::cerr << "(r,c): (" << r << "," << c << "), colorindex is " << cindex << std::endl; 
         drawCube((float)c,(float)r,0.0,cindex);
       }
    }
  }
}

void Viewer::drawWall()
{
  for ( int i = 0; i < 12; ++i )
  {
      drawCube((float)i-1.0,-1.0,0.0,7);
  }
  for ( int i = 1; i < 21; ++i )
  {
      drawCube(-1.0,(float)i-1.0,0.0,7);
      drawCube(10.0,(float)i-1.0,0.0,7);
  }
}

void Viewer::rotate_image(bool useData, float data)
{
  float howMuch_angle[4];
  for ( int i = 1; i < 4; ++i ) howMuch_angle[i] = 0.0;
  if ( ! m_shift && useData ) //rotation
  {
    for ( int i = 1; i < 4; ++i )
    {
      if ( m_button_number[i] ) howMuch_angle[i] = data - m_button_press_angle[i];
    }
  }
 
 //when useData is off, it still has to rotate based on the previous record
 //by doing this once, it updates all rotations and howMuch_angle is 0
  if ( !useData )
  {
      glRotatef(m_angle[0], 1.0f, 0.0f, 0.0f);
      glRotatef(m_angle[1], 0.0f, 1.0f, 0.0f);
      glRotatef(m_angle[2], 0.0f, 0.0f, 1.0f);
  }
  if ( m_button_number[1] )
  {
      std::cerr << "rotate: button 1" << std::endl;
      glRotatef(howMuch_angle[1]+m_angle[0], 1.0f, 0.0f, 0.0f);
      glRotatef(m_angle[1], 0.0f, 1.0f, 0.0f);
      glRotatef(m_angle[2], 0.0f, 0.0f, 1.0f);
      m_angle[0] += howMuch_angle[1];
  }
  if ( m_button_number[2] )
  {
      std::cerr << "rotate: button 2" <<std::endl;
      glRotatef(m_angle[0], 1.0f, 0.0f, 0.0f);
      glRotatef(howMuch_angle[2]+m_angle[1], 0.0f, 1.0f, 0.0f);
      glRotatef(m_angle[2], 0.0f, 0.0f, 1.0f);
      m_angle[1] += howMuch_angle[2];
  }
  if ( m_button_number[3] )
  {
      std::cerr << "rotate: button 3" <<std::endl;
      glRotatef(m_angle[0], 1.0f, 0.0f, 0.0f);
      glRotatef(m_angle[1], 0.0f, 1.0f, 0.0f);
      glRotatef(howMuch_angle[3]+m_angle[2], 0.0f, 0.0f, 1.0f);
      m_angle[2] += howMuch_angle[3];
  }

}

void Viewer::scale_image(bool useData, float data)
{
  float howMuch_scale = 1.0;
  if ( m_shift && m_button_press_scale > data && useData) // scale down
  {
    howMuch_scale = std::max ( 0.01, 1 - ( m_button_press_scale - data ) / 100.0 );
    m_scale = ( howMuch_scale * m_scale > SCALE_MIN ? m_scale * howMuch_scale :
                                          SCALE_MIN );
  }
  else if ( m_shift && data != m_button_press_scale && useData)// scale up
  {
    howMuch_scale = 1.0 + ( data - m_button_press_scale ) / 50.0;
    m_scale = ( howMuch_scale * m_scale < SCALE_MAX ? m_scale * howMuch_scale :
                                          SCALE_MAX );
  }
  glScalef(m_scale,m_scale,m_scale);

  if ( useData )
  {
    m_button_press_scale = data;
    for ( int i = 1; i < 4; ++i )
    {
      if ( m_button_number[i] ) m_button_press_angle[i] = data;
    }
  }
}
