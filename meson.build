project('expidus-launcher', 'c', license: 'GPL-3.0-only', version: '0.1.0-prealpha')

prefix = get_option('prefix')
bindir = prefix / get_option('bindir')
datadir = prefix / get_option('datadir')
libdir = prefix / get_option('libdir')
libexecdir = prefix / get_option('libexecdir')
includedir = prefix / get_option('includedir')

longver = '@0@'.format(meson.project_version())
shortver = longver
git = find_program('git', native: true, required: false)
conf_data = configuration_data()
build_user = get_option('build-user')

if git.found()
  git_commit = run_command(git, 'rev-parse', '--short', 'HEAD', check: false)
  git_branch = run_command(git, 'rev-parse', '--abbrev-ref', 'HEAD', check: false)
  git_config_username = run_command(git, 'config', '--get', 'user.name', check: false)
  git_config_email = run_command(git, 'config', '--get', 'user.email', check: false)

  if git_commit.returncode() == 0
    git_commit = git_commit.stdout().strip()
  else
    git_commit = get_option('git-commit')
  endif

  if git_branch.returncode() == 0
    git_branch = git_branch.stdout().strip()
  else
    git_branch = get_option('git-branch')
  endif

  if git_config_username.returncode() == 0 and git_config_email.returncode() == 0
    git_config_username = git_config_username.stdout().strip()
    git_config_email = git_config_email.stdout().strip()
    build_user = '@0@ (@1@)'.format(git_config_username, git_config_email)
  endif
else
  git_commit = get_option('git-commit')
  git_branch = get_option('git-branch')
endif

shortver = '@0@-@1@'.format(meson.project_version(), git_commit)
longver = '@0@ (branch \'@1@\')'.format(shortver, git_branch)
version = shortver.split('-')[0]

if get_option('buildtype') == 'release'
  shortver = meson.project_version()
endif

plymouth_boot_client = dependency('ply-boot-client', required: get_option('plymouth').disable_auto_if(not dependency('ply-boot-client', required: false).found()))
