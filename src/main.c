
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sys/utsname.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "shell.h"
#include "utils.h"
#ifndef KSH_VERSION
#define KSH_VERSION "0.1.1"
#endif
#ifndef KSH_BUILD_DATE
#define KSH_BUILD_DATE "unknown"
#endif
#ifndef KSH_TARGET
#define KSH_TARGET "unknown"
#endif

struct i18n_strings {
    const char *code; /* normalized code */
    const char *first_line; /* format: product/version/triplet */
    const char *copyright;
    const char *license;
    const char *free_text;
    const char *warranty;
    const char *build_label; /* format with %s */
    const char *target_label; /* format with %s */
};

static const struct i18n_strings translations[] = {
    /* English */
    {"en", "kzsh, version %s(1)-release (%s)",
     "Copyright (C) %s Kuznix",
     "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>",
     "This is free software; you can redistribute it and/or modify it.",
     "There is NO WARRANTY, to the extent permitted by law.",
     "Build date: %s", "Target: %s"},
    /* Polish (pl) - keep as default localized we had */
    {"pl", "kzsh, wersja %s(1)-release (%s)",
     "Copyright (C) %s Kuznix",
     "Licencja GPLv3+: GNU GPL wersja 3 lub późniejsza <http://gnu.org/licenses/gpl.html>",
     "To oprogramowanie jest wolnodostępne; można je swobodnie zmieniać i rozpowszechniać.",
     "Nie ma ŻADNEJ GWARANCJI w granicach dopuszczanych przez prawo.",
     "Data kompilacji: %s", "Cel: %s"},
    /* Spanish */
    {"es", "kzsh, versión %s(1)-release (%s)",
     "Copyright (C) %s Kuznix",
     "Licencia GPLv3+: GNU GPL versión 3 o posterior <http://gnu.org/licenses/gpl.html>",
     "Este software es libre; puede modificarlo y redistribuirlo.",
     "NO HAY NINGUNA GARANTÍA, en la medida permitida por la ley.",
     "Fecha de compilación: %s", "Objetivo: %s"},
    /* Japanese (ja) */
    {"ja", "kzsh、バージョン %s(1)-release (%s)",
     "Copyright (C) %s Kuznix",
     "ライセンス GPLv3+: GNU GPL バージョン3以降 <http://gnu.org/licenses/gpl.html>",
     "本ソフトウェアはフリーソフトウェアです。自由に改変および再配布できます。",
     "法律で許される範囲で、保証は一切ありません。",
     "ビルド日: %s", "ターゲット: %s"},
    /* Chinese (zh) - simplified */
    {"zh", "kzsh，版本 %s(1)-release (%s)",
     "Copyright (C) %s Kuznix",
     "许可证 GPLv3+: GNU GPL 第3版或更高版本 <http://gnu.org/licenses/gpl.html>",
     "本软件是自由软件；您可以自由修改和传播。",
     "在法律允许的范围内，不提供任何担保。",
     "构建日期: %s", "目标: %s"},
    /* Russian */
    {"ru", "kzsh, версия %s(1)-release (%s)",
     "Copyright (C) %s Kuznix",
     "Лицензия GPLv3+: GNU GPL версия 3 или более поздняя <http://gnu.org/licenses/gpl.html>",
     "Это свободное программное обеспечение; вы можете свободно изменять и распространять его.",
     "ОТСУТСТВУЕТ КАКАЯ-ЛИБО ГАРАНТИЯ в пределах, допустимых законом.",
     "Дата сборки: %s", "Цель: %s"},
    /* German */
    {"de", "kzsh, Version %s(1)-release (%s)",
     "Copyright (C) %s Kuznix",
     "Lizenz GPLv3+: GNU GPL Version 3 oder später <http://gnu.org/licenses/gpl.html>",
     "Diese Software ist freie Software; Sie können sie verändern und weitergeben.",
     "KEINE GARANTIE, soweit gesetzlich zulässig.",
     "Build-Datum: %s", "Ziel: %s"},
    /* French */
    {"fr", "kzsh, version %s(1)-release (%s)",
     "Copyright (C) %s Kuznix",
     "Licence GPLv3+: GNU GPL version 3 ou ultérieure <http://gnu.org/licenses/gpl.html>",
     "Ce logiciel est libre ; vous pouvez le modifier et le redistribuer.",
     "AUCUNE GARANTIE, dans les limites autorisées par la loi.",
     "Date de compilation: %s", "Cible: %s"},
    /* Portuguese (pt and pt-br) */
    {"pt", "kzsh, versão %s(1)-release (%s)",
     "Copyright (C) %s Kuznix",
     "Licença GPLv3+: GNU GPL versão 3 ou posterior <http://gnu.org/licenses/gpl.html>",
     "Este programa é software livre; você pode modificá-lo e redistribuí-lo.",
     "NÃO HÁ GARANTIA, na extensão permitida por lei.",
     "Data da compilação: %s", "Alvo: %s"},
    /* fallback English */
    {"", "kzsh, version %s(1)-release (%s)",
     "Copyright (C) %s Kuznix",
     "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>",
     "This is free software; you can redistribute it and/or modify it.",
     "There is NO WARRANTY, to the extent permitted by law.",
     "Build date: %s", "Target: %s"}
};

static const struct i18n_strings *find_translation(const char *code) {
    if (!code) return &translations[0];
    for (size_t i = 0; i < sizeof(translations)/sizeof(translations[0]); ++i) {
        if (translations[i].code && translations[i].code[0] && strcmp(translations[i].code, code) == 0)
            return &translations[i];
    }
    /* not found -> fallback to English (index 0) */
    return &translations[0];
}

static void normalize_locale(const char *env, char *out, size_t outlen) {
    /* env may be like "en_US.UTF-8" or "pt_BR"; normalize to two-letter code or pt-br */
    if (!env || !env[0]) { out[0] = '\0'; return; }
    char buf[64];
    size_t i;
    for (i = 0; i < sizeof(buf)-1 && env[i]; ++i) buf[i] = (char) tolower((unsigned char)env[i]);
    buf[i] = '\0';
    /* handle pt_br / pt-br specially */
    if (strncmp(buf, "pt_br", 5) == 0 || strncmp(buf, "pt-br", 5) == 0) {
        strncpy(out, "pt", outlen-1);
        out[outlen-1] = '\0';
        return;
    }
    /* take prefix before '_' or '.' or '-' */
    char *sep = strpbrk(buf, "_.-");
    if (sep) *sep = '\0';
    /* map zh* -> zh, ja* -> ja, ru*, de*, fr*, es*, en*, pt* -> pt */
    if (strlen(buf) >= 2) {
        out[0] = buf[0]; out[1] = buf[1]; out[2] = '\0';
    } else {
        out[0] = '\0';
    }
}

void print_version(void) {
    const char *version = KSH_VERSION ? KSH_VERSION : "0.0";
    const char *build_date = KSH_BUILD_DATE;
    const char *target = KSH_TARGET;

    /* build-date fallback */
    if (!build_date || strchr(build_date, '$') != NULL || strcmp(build_date, "unknown") == 0) {
        const char *d = __DATE__;
        char month[4]; int day, year;
        if (sscanf(d, "%3s %d %d", month, &day, &year) == 3) {
            static const char *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
            char monnum[3] = "00"; const char *p = strstr(months, month);
            if (p) { int idx = (p - months)/3 + 1; snprintf(monnum, sizeof(monnum), "%02d", idx); }
            static char buf[16]; snprintf(buf, sizeof(buf), "%04d-%s-%02d", year, monnum, day); build_date = buf;
        } else {
            time_t t = time(NULL); struct tm tm = *localtime(&t); static char buf2[16]; strftime(buf2, sizeof(buf2), "%Y-%m-%d", &tm); build_date = buf2;
        }
    }

    /* target/triplet fallback */
    static char triplet[128];
    if (!target || strchr(target, '$') != NULL || strcmp(target, "unknown") == 0) {
        struct utsname uts;
        if (uname(&uts) == 0) {
            char osname[64]; size_t i;
            for (i = 0; i < sizeof(osname)-1 && uts.sysname[i]; ++i) osname[i] = (char) tolower((unsigned char)uts.sysname[i]);
            osname[i] = '\0';
            snprintf(triplet, sizeof(triplet), "%s-pc-%s-gnu", uts.machine, osname);
            target = triplet;
        } else {
            target = "unknown";
        }
    }

    /* pick language from environment variables */
    char locale[8]; locale[0] = '\0';
    const char *envs[] = { getenv("LC_ALL"), getenv("LC_MESSAGES"), getenv("LANG"), NULL };
    for (size_t i = 0; envs[i]; ++i) { if (envs[i] && envs[i][0]) { normalize_locale(envs[i], locale, sizeof(locale)); break; } }

    const struct i18n_strings *tr = NULL;
    if (locale[0]) tr = find_translation(locale);
    if (!tr) tr = &translations[0];

    /* print localized output */
    char firstbuf[256];
    snprintf(firstbuf, sizeof(firstbuf), tr->first_line, version, target);
    printf("%s\n", firstbuf);
    /* Determine copyright year (prefer configure-time macros if present) */
#ifdef KZSH_COPYRIGHT_YEAR
    const char *cfg_year = KZSH_COPYRIGHT_YEAR;
#elif defined(KSH_COPYRIGHT_YEAR)
    const char *cfg_year = KSH_COPYRIGHT_YEAR;
#else
    const char *cfg_year = NULL;
#endif
    char yearbuf[8];
    if (cfg_year && cfg_year[0] && strstr(cfg_year, "${") == NULL) {
        strncpy(yearbuf, cfg_year, sizeof(yearbuf)-1);
        yearbuf[sizeof(yearbuf)-1] = '\0';
    } else {
        time_t t = time(NULL); struct tm tm = *localtime(&t); strftime(yearbuf, sizeof(yearbuf), "%Y", &tm);
    }
    printf(tr->copyright, yearbuf);
    /* ensure the copyright line ends with a newline before printing the license */
    printf("\n");
    printf("%s\n\n", tr->license);
    printf("%s\n", tr->free_text);
    printf("%s\n", tr->warranty);
}

void print_help(void) {
    printf("Usage: kzsh [options]\n");
    printf("Options:\n");
    printf("  --version     Show version info\n");
    printf("  --help        Show this help message\n");
    printf("  -c <command>  Execute command\n");
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "--version") == 0) {
            print_version();
            return 0;
        } else if (strcmp(argv[1], "--help") == 0) {
            print_help();
            return 0;
        } else if (strcmp(argv[1], "-c") == 0 && argc > 2) {
            printf("Executing: %s\n", argv[2]);
            /* TODO: Execute command */
            return 0;
        }
    }

    /* Export version info for tools like fastfetch/neofetch/screenfetch. */
    setenv("KSH_VERSION", KSH_VERSION, 1);
    setenv("KSH_BUILD_DATE", KSH_BUILD_DATE, 1);
    setenv("KSH_TARGET", KSH_TARGET, 1);

    /* Start the interactive shell without printing a banner. */
    shell_start(KSH_VERSION);

    /* TODO: implement shell main loop */
    return 0;
}
