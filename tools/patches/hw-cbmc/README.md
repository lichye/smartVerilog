# Patches applied to the downloaded `hw-cbmc` dependency

Applied by `tools/apply-hw-cbmc-patches.sh` while building `.deps/hw-cbmc`.
The script is idempotent, and the build wrapper restores newly applied patches
after compilation so the dependency checkout remains clean.

## 0001-verilog-keep-single-attribute-instances.patch

**Upstream bug.** In `src/verilog/parser.y`, the base case of `attr_spec_list`
is

```
attr_spec_list:
	  attr_spec
	  	{ init($$); }        <-- $1 is discarded
```

`init($$)` allocates a *fresh, empty* list and never moves `$1` into it, so a
single-attribute instance — `(* anyseq *) reg x;` — parses to an **empty**
attribute list, and `add_attributes()` then drops it entirely (it only stores
non-empty lists). With two or more attributes, `(* a, b *)`, only the trailing
ones survive.

Consequence for SMART: `(* anyseq *)` / `(* anyconst *)` free registers are
invisible in the parse tree, which is precisely the information the frontend
adapter (WP2) needs from hw-cbmc. Verified on `ebmc-5.6`, `ebmc-6.0` and
`main` (1a4ffdb) — the bug is present in all of them.

The fix is one line: move `$1` into the fresh list.

Retire this patch once it is merged upstream and `HWCBMC_REV` in `install.sh`
is bumped past the merge commit.
