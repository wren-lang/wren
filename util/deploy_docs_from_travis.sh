#!/bin/bash

set -e

make gh-pages
# TODO: This strips the syntax highlighting because the custom pygments lexer
# isn't installed.

git clone https://${GH_TOKEN}@github.com/${TRAVIS_REPO_SLUG} gh-pages-repo
cd gh-pages-repo
git checkout gh-pages

rm -rf *
cp -r ../build/gh-pages/* .

# TODO: Restore CNAME file that gets deleted by `rm -rf *`.

git status
ls

if ! $( git diff-index --quiet HEAD ) ; then
	git config user.name "Travis CI"
	git config user.email "$COMMIT_AUTHOR_EMAIL"
	git add --all .
	git commit -m "Deploy to GitHub Pages: ${SHA}"
	git push
fi
