/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*-
 *
 * Copyright (C) 2014 James Antil <james@fedoraproject.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2 of the licence or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include "rpmostree-db-builtins.h"
#include "rpmostree-libbuiltin.h"
#include "rpmostree-rpm-util.h"

static char *opt_format;
static gboolean opt_changelogs;

static GOptionEntry option_entries[] = {
  { "format", 'F', 0, G_OPTION_ARG_STRING, &opt_format, "Output format: \"diff\" or (default) \"block\"", "FORMAT" },
  { "changelogs", 'c', 0, G_OPTION_ARG_NONE, &opt_changelogs, "Also output RPM changelogs", NULL },
  { NULL }
};

int
rpmostree_db_builtin_diff (int argc, char **argv,
                           RpmOstreeCommandInvocation *invocation,
                           GCancellable *cancellable, GError **error)
{
  g_autoptr(GOptionContext) context =
    g_option_context_new ("COMMIT COMMIT - Show package changes between two commits");

  g_autoptr(OstreeRepo) repo = NULL;
  if (!rpmostree_db_option_context_parse (context, option_entries, &argc, &argv, invocation, &repo,
                                          cancellable, error))
    return EXIT_FAILURE;

  if (argc != 3)
    {
      g_autofree char *message = NULL;

      message = g_strdup_printf ("\"%s\" takes exactly 2 arguments",
                                 g_get_prgname ());
      rpmostree_usage_error (context, message, error);
      return EXIT_FAILURE;
    }

  g_autoptr(RpmRevisionData) rpmrev1 = NULL;
  if (!(rpmrev1 = rpmrev_new (repo, argv[1], NULL, cancellable, error)))
    return EXIT_FAILURE;

  g_autoptr(RpmRevisionData) rpmrev2 = NULL;
  if (!(rpmrev2 = rpmrev_new (repo, argv[2], NULL, cancellable, error)))
    return EXIT_FAILURE;

  if (!g_str_equal (argv[1], rpmrev_get_commit (rpmrev1)))
    printf ("ostree diff commit old: %s (%s)\n", argv[1], rpmrev_get_commit (rpmrev1));
  else
    printf ("ostree diff commit old: %s\n", argv[1]);

  if (!g_str_equal (argv[2], rpmrev_get_commit (rpmrev2)))
    printf ("ostree diff commit new: %s (%s)\n", argv[2], rpmrev_get_commit (rpmrev2));
  else
    printf ("ostree diff commit new: %s\n", argv[2]);

  if (opt_format == NULL)
    opt_format = "block";

  if (g_str_equal (opt_format, "diff"))
    {
      rpmhdrs_diff_prnt_diff (rpmhdrs_diff (rpmrev_get_headers (rpmrev1),
                                            rpmrev_get_headers (rpmrev2)));
    }
  else if (g_str_equal (opt_format, "block"))
    {
      rpmhdrs_diff_prnt_block (opt_changelogs,
                               rpmhdrs_diff (rpmrev_get_headers (rpmrev1),
                                             rpmrev_get_headers (rpmrev2)));
    }
  else
    {
      glnx_throw (error, "Format argument is invalid, pick one of: diff, block");
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

