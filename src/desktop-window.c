/*
 * This file is part of budgie-desktop-icons.
 *
 * Copyright © 2018 Budgie Desktop Developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define _GNU_SOURCE

#include "util.h"

#include "desktop-view.h"
#include "desktop-window.h"

#include <gtk/gtk.h>

struct _BudgieDesktopWindowClass {
        GtkApplicationWindowClass parent_class;
};

/**
 * BudgieDesktopWindow is used to manage the display and appropriately construct
 * windows per output.
 */
struct _BudgieDesktopWindow {
        GtkApplicationWindow parent;

        GtkWidget *view; /** Our main view */

        GdkMonitor *monitor; /** Our output */
        GdkDisplay *display; /** Our display */
        GdkScreen *screen;   /** Our screen */
};

G_DEFINE_TYPE(BudgieDesktopWindow, budgie_desktop_window, GTK_TYPE_WINDOW)

enum { PROP_MONITOR = 1, N_PROPS };

static GParamSpec *obj_properties[N_PROPS] = {
        NULL,
};

static void budgie_desktop_window_set_property(GObject *object, guint id, const GValue *value,
                                               GParamSpec *spec)
{
        BudgieDesktopWindow *self = BUDGIE_DESKTOP_WINDOW(object);

        switch (id) {
        case PROP_MONITOR:
                self->monitor = g_value_get_pointer(value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

static void budgie_desktop_window_get_property(GObject *object, guint id, GValue *value,
                                               GParamSpec *spec)
{
        BudgieDesktopWindow *self = BUDGIE_DESKTOP_WINDOW(object);

        switch (id) {
        case PROP_MONITOR:
                g_value_set_pointer(value, self->monitor);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

/**
 * budgie_desktop_window_new:
 *
 * Construct a new BudgieDesktopWindow object
 */
GtkWidget *budgie_desktop_window_new(GApplication *app, GdkMonitor *monitor)
{
        return g_object_new(BUDGIE_TYPE_DESKTOP_WINDOW,
                            "application",
                            app,
                            "type",
                            GTK_WINDOW_TOPLEVEL,
                            "monitor",
                            monitor,
                            NULL);
}

/**
 * budgie_desktop_window_dispose:
 *
 * Clean up a BudgieDesktopWindow instance
 */
static void budgie_desktop_window_dispose(GObject *obj)
{
        BudgieDesktopWindow *self = NULL;
        self = BUDGIE_DESKTOP_WINDOW(obj);

        G_OBJECT_CLASS(budgie_desktop_window_parent_class)->dispose(obj);
}

/**
 * Handle constructor specifics for our Window so we can align ourself with
 * the geometry of the output.
 */
static void budgie_desktop_window_constructed(GObject *obj)
{
        BudgieDesktopWindow *self = NULL;
        GdkRectangle display_area = { 0 };
        GdkVisual *rgba_visual = NULL;

        /* Stash for later use */
        self = BUDGIE_DESKTOP_WINDOW(obj);
        self->display = gdk_monitor_get_display(self->monitor);
        /* GTK 3.22.x only has a single GdkScreen per display now */
        self->screen = gdk_display_get_default_screen(self->display);

        g_message("New output on %s %s",
                  gdk_monitor_get_manufacturer(self->monitor),
                  gdk_monitor_get_model(self->monitor));

        /* Resize to available geometry */
        gdk_monitor_get_workarea(self->monitor, &display_area);
        gtk_window_move(GTK_WINDOW(self), display_area.x, display_area.y);
        gtk_window_set_default_size(GTK_WINDOW(self), display_area.width, display_area.height);

        /* Adopt RGBA visual if possible */
        rgba_visual = gdk_screen_get_rgba_visual(self->screen);
        if (rgba_visual) {
                gtk_widget_set_visual(GTK_WIDGET(self), rgba_visual);
        } else {
                g_warning("RGBA Compositing missing on display %s %s",
                          gdk_monitor_get_manufacturer(self->monitor),
                          gdk_monitor_get_model(self->monitor));
        }

        G_OBJECT_CLASS(budgie_desktop_window_parent_class)->constructed(obj);
}

/**
 * budgie_desktop_window_class_init:
 *
 * Handle class initialisation
 */
static void budgie_desktop_window_class_init(BudgieDesktopWindowClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = budgie_desktop_window_dispose;
        obj_class->set_property = budgie_desktop_window_set_property;
        obj_class->get_property = budgie_desktop_window_get_property;
        obj_class->constructed = budgie_desktop_window_constructed;

        obj_properties[PROP_MONITOR] = g_param_spec_pointer("monitor",
                                                            "The GdkMonitor",
                                                            "Corresponding GdkMonitor",
                                                            G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
        g_object_class_install_properties(obj_class, N_PROPS, obj_properties);
}

/**
 * budgie_desktop_window_init:
 *
 * Handle construction of the BudgieDesktopWindow
 */
static void budgie_desktop_window_init(BudgieDesktopWindow *self)
{
        GtkWindow *window = GTK_WINDOW(self);

        gtk_window_set_decorated(window, FALSE);
        gtk_window_set_skip_pager_hint(window, TRUE);
        gtk_window_set_skip_taskbar_hint(window, TRUE);
        gtk_window_set_type_hint(window, GDK_WINDOW_TYPE_HINT_DESKTOP);
        gtk_window_set_title(window, "Desktop Window");
        gtk_window_set_resizable(window, FALSE);

        self->view = budgie_desktop_view_new();
        gtk_container_add(GTK_CONTAINER(self), self->view);

        /* We're an RGBA window */
        gtk_widget_set_app_paintable(GTK_WIDGET(self), TRUE);
}

/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 8
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=8 tabstop=8 expandtab:
 * :indentSize=8:tabSize=8:noTabs=true:
 */
