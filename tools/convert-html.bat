@echo off
setlocal

set "SEARCH_DIR=../build/coverage"

:: HTML Tidy: http://api.html-tidy.org/tidy/tidylib_api_5.2.0/tidy_cmd.html

for /r "%SEARCH_DIR%" %%F in (*.html) do (
    tidy -asxml -quiet -o "%%~dpnF.xml" "%%F"
)

endlocal
