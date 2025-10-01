#ifndef BUILTINS_LIST_H
#define BUILTINS_LIST_H

// List of supported builtins (expand as needed)
#define BUILTIN_COMMANDS \
    X(echo) \
    X(true) \
    X(false) \
    X(exit) \
    X(cd) \
    X(pwd) \
    X(ls) \
    X(cat) \
    X(chmod) \
    X(chown) \
    X(mkdir) \
    X(rmdir) \
    X(rm) \
    X(mv) \
    X(cp) \
    X(date) \
    X(sleep) \
    X(printenv) \
    X(set) \
    X(unset) \
    X(export) \
    X(which) \
    X(type) \
    X(umask) \
    X(ln) \
    X(test) \
    X(printf) \
    X(head) \
    X(tail) \
    X(wc) \
    X(grep) \
    X(fgrep) \
    X(egrep) \
    X(find) \
    X(diff) \
    X(sort) \
    X(tr) \
    X(uniq) \
    X(xargs) \
    X(seq) \
    X(yes) \
    X(whoami) \
    X(who) \
    X(id) \
    X(logname) \
    X(groups) \
    X(tee) \
    X(stty) \
    X(clear) \
    X(history) \
    X(alias) \
    X(unalias) \
    X(help)

#endif // BUILTINS_LIST_H
