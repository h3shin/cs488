#ifndef CS488_VIEWER_HPP
#define CS488_VIEWER_HPP

#include <gtkmm.h>
#include <gtkglmm.h>
#include "game.hpp"

// The "main" OpenGL widget
class Viewer : public Gtk::GL::DrawingArea {
public:
  Viewer();
  virtual ~Viewer();

  // A useful function that forces this widget to rerender. If you
  // want to render a new frame, do not call on_expose_event
  // directly. Instead call this, which will cause an on_expose_event
  // call when the time is right.
  void invalidate();
  enum ColorMode {
    WIRE_FRAME,
    FACE,
    MULTICOLOR,
    CHRISTMAS
  };
  enum SpeedMode {
    SLOW,
    MEDIUM,
    FAST
  };

  void set_color_mode(ColorMode mode) { m_color_mode = mode; invalidate(); }
  void set_buffer_mode(int mode) { m_doublebuffer *= mode; invalidate(); }
  void set_speed_mode(SpeedMode mode);
  void shift_set(bool val) { m_shift = val; }
  void new_game();
  void reset_game();
  Game* get_game();

protected:

  // Events we implement
  // Note that we could use gtkmm's "signals and slots" mechanism
  // instead, but for many classes there's a convenient member
  // function one just needs to define that'll be called with the
  // event.

  // Called when GL is first initialized
  virtual void on_realize();
  // Called when our window needs to be redrawn
  virtual bool on_expose_event(GdkEventExpose* event);
  // Called when the window is resized
  virtual bool on_configure_event(GdkEventConfigure* event);
  // Called when a keyboard key is pressed
  virtual bool on_button_press_event(GdkEventButton* event);
  // Called when a mouse button is released
  virtual bool on_button_release_event(GdkEventButton* event);
  // Called when the mouse moves
  virtual bool on_motion_notify_event(GdkEventMotion* event);

  void drawCube(float x, float y, float z, int cindex);
  void drawVertex(int posn, float x, float y, float z);
  bool render_image();
  void rotate_image();
  void scale_image();
  void drawPieces();
  void drawWall();
  bool timeout_handler();

private:

  //m_button_number<*,0> if not pressed 
  //m_button_number<*,1> pressed but direction non-specified
  //m_button_number<*,2> pressed and negative direction
  //m_button_number<*,3> pressed and positive direction
  //m_button_number<*,4> released and direction non-specified
  //m_button_number<*,5> released and negative direction
  //m_button_number<*,6> released and positive direction
  int m_button_number[4][1]; //[0][*] not used
  ColorMode m_color_mode;
  SpeedMode m_speed_mode;
  float m_button_press_angle[4]; //[0] not used
  float m_button_press_scale;
  float m_angle[3];
  bool m_shift;
  float m_scale;
  int m_doublebuffer;
  Game *m_game;
  float m_color[8][3];
  bool m_disconnect;
  int m_speed[3];
  bool m_gameover;
};

#endif
