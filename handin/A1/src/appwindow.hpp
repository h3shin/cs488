#ifndef APPWINDOW_HPP
#define APPWINDOW_HPP

#include <gtkmm.h>
#include "viewer.hpp"
#include "game.hpp"

class AppWindow : public Gtk::Window {
public:
  AppWindow();
  
protected:
  virtual bool on_key_press_event( GdkEventKey *ev );
  virtual bool on_key_release_event( GdkEventKey *ev );

private:
  // A "vertical box" which holds everything in our window
  Gtk::VBox m_vbox;

  // The menubar, with all the menus at the top of the window
  Gtk::MenuBar m_menubar;

  // Each menu itself
  Gtk::Menu m_menu_app;

  Gtk::Menu m_menu_drawmode;
  Gtk::Menu m_menu_speed;
  Gtk::Menu m_menu_buffering;
  Gtk::RadioButtonGroup m_draw_group;
  Gtk::RadioButtonGroup m_speed_group;

  // The main OpenGL area
  Viewer m_viewer;
  Game *m_game;
};

#endif
