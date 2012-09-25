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
//             Gdk::KEY_RELEASE_MASK    | //Check
             Gdk::VISIBILITY_NOTIFY_MASK);
  m_color_mode = WIRE_FRAME;
  m_angle[0] = m_angle[1] = m_angle[2] = 0.0;
  m_shift = false;
  m_scale = 1.0;
}

Viewer::~Viewer()
{
  // Nothing to do here right now.
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
  glClearColor(0.7, 0.7, 1.0, 0.0);
//  glClearColor(1.0, 1.0, 1.0, 0.0);
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

void Viewer::drawCube(float x, float y, float z)
{
  if (m_color_mode == WIRE_FRAME)
  {
    glBegin(GL_LINES);
    drawVertex(1, x, y, z); drawVertex(3, x, y, z);

    drawVertex(2, x, y, z); drawVertex(4, x, y, z);

    drawVertex(6, x, y, z); drawVertex(8, x, y, z);

    drawVertex(5, x, y, z); drawVertex(7, x, y, z);

    glNormal3f(0.0f, 0.0f, 0.0f);
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

  }
  else // if (m_color_mode == FACE)  TODO
  {
    glBegin(GL_QUADS);

    //Front
    glNormal3f(0.0f, 0.0f, 1.0f);
    drawVertex(1, x, y, z); drawVertex(2, x, y, z);
    drawVertex(4, x, y, z); drawVertex(3, x, y, z);

    //Back
    glNormal3f(0.0f, 0.0f, -1.0f);
    drawVertex(5, x, y, z); drawVertex(6, x, y, z);
    drawVertex(8, x, y, z); drawVertex(7, x, y, z);

    //Left
    glNormal3f(-1.0f, 0.0f, 0.0f);
    drawVertex(1, x, y, z); drawVertex(5, x, y, z);
    drawVertex(7, x, y, z); drawVertex(3, x, y, z);

    //Right
    glNormal3f(1.0f, 0.0f, -0.6f);
    drawVertex(6, x, y, z); drawVertex(2, x, y, z);
    drawVertex(4, x, y, z); drawVertex(8, x, y, z);

    //Bottom
    glNormal3f(0.0f, 1.0f, 0.0f);
    drawVertex(7, x, y, z); drawVertex(3, x, y, z);
    drawVertex(4, x, y, z); drawVertex(8, x, y, z);

    //Top
    glNormal3f(0.0f, -1.0f, 0.5f);
    drawVertex(5, x, y, z); drawVertex(1, x, y, z);
    drawVertex(2, x, y, z); drawVertex(6, x, y, z);
  }
  glEnd();
}

void Viewer::new_game()
{
  m_color_mode = WIRE_FRAME;
}

void Viewer::reset_game()
{

}

bool Viewer::on_expose_event(GdkEventExpose* event)
{
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
/*
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
*/
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

  return true;
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

  m_button_number = event->button;
  m_button_press_scale = m_button_press_angle = event->x;
  return true;
}

bool Viewer::on_button_release_event(GdkEventButton* event)
{
  std::cerr << "Stub: Button " << event->button << " released" << std::endl;
  return true;
}

bool Viewer::on_motion_notify_event(GdkEventMotion* event)
{
  std::cerr << "Stub: Motion at " << event->x << ", " << event->y << std::endl;
  m_motion_notify = event->x;
  Glib::RefPtr<Gdk::GL::Drawable> gldrawable = get_gl_drawable();

  if (!gldrawable) return false;

  if (!gldrawable->gl_begin(get_gl_context()))
    return false;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glTranslated(0.0, 0.0, -40.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_LIGHTING);
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

  float howMuch_angle = 0.0;
  float howMuch_scale = 1.0;

  if ( ! m_shift ) //rotation
  {
    howMuch_angle = m_motion_notify - m_button_press_angle;
  }

  // Rotate
  switch(m_button_number)
  {
    case 1: //rotate around x-axis
      std::cerr << "rotate: button 1" << std::endl;
      glRotatef(howMuch_angle+m_angle[0], 1.0f, 0.0f, 0.0f);
      glRotatef(m_angle[1], 0.0f, 1.0f, 0.0f);
      glRotatef(m_angle[2], 0.0f, 0.0f, 1.0f);
      m_angle[0] += howMuch_angle;
    break;
    case 2: //rotate around y-axis
      std::cerr << "rotate: button 2" <<std::endl;
      glRotatef(m_angle[0], 1.0f, 1.0f, 0.0f);
      glRotatef(howMuch_angle+m_angle[1], 0.0f, 1.0f, 0.0f);
      glRotatef(m_angle[2], 0.0f, 0.0f, 1.0f);
      m_angle[1] += howMuch_angle;
    break;
    default: //rotate around z-axis
      std::cerr << "rotate: button 3" <<std::endl;
      glRotatef(m_angle[0], 1.0f, 0.0f, 0.0f);
      glRotatef(m_angle[1], 0.0f, 1.0f, 0.0f);
      glRotatef(howMuch_angle+m_angle[2], 0.0f, 0.0f, 1.0f);
      m_angle[2] += howMuch_angle;
  }

  // Scale
  if ( m_shift && m_button_press_scale > m_motion_notify ) // scale down
  {
    howMuch_scale = std::max ( 0.01, 1 - ( m_button_press_scale - m_motion_notify ) / 100.0 );
    m_scale = ( howMuch_scale * m_scale > SCALE_MIN ? m_scale * howMuch_scale :
                                          SCALE_MIN );
  }
  else if ( m_shift && m_motion_notify != m_button_press_scale )// scale up
  {
    howMuch_scale = 1.0 + ( m_motion_notify - m_button_press_scale ) / 50.0;
    m_scale = ( howMuch_scale * m_scale < SCALE_MAX ? m_scale * howMuch_scale :
                                          SCALE_MAX );
  }
//  glScalef(howMuch_scale, howMuch_scale, howMuch_scale);
  glScalef(m_scale,m_scale,m_scale);

  m_button_press_angle = m_button_press_scale = m_motion_notify;
  std::cerr << "final m_scale: " << m_scale << std::endl;

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
  //TODO: Redraw all the "pieces"

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  // Swap the contents of the front and back buffers so we see what we
  // just drew. This should only be done if double buffering is enabled.
  gldrawable->swap_buffers();

  gldrawable->gl_end();

  return true;
}
