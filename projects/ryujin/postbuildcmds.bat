@ECHO OFF
SET input_dir=%1
SET output_dir=%2
SET build_cfg=%3

IF [%3]==[] (
    SET build_cfg=""
)

:: Create the output directory
IF NOT EXIST %output_dir% (
    MD %output_dir
)

:: Execute glslc on each
FOR /F "delims=" %%a IN ('dir "%input_dir%" /b /s') DO (
    FOR %%G IN (.vert,
                .vs,
                .tcs,
                .tesc,
                .tes,
                .tese,
                .geom,
                .gs,
                .frag,
                .fs,
                .comp,
                .cs) DO (
                    IF /I "%%~xa"=="%%~G" (
                        IF %build_cfg%==debug (
                            glslc "%%~a" -o "%%~a.spv" -O0
                        ) ELSE (
                            IF %build_cfg%==release (
                                glslc "%%~a" -o "%%~a.spv" -O
                            ) ELSE (
                                glslc "%%~a" -o "%%~a.spv"
                            )
                        )
                    )
                )
)

:: Copy data file structure
ROBOCOPY %input_dir% %output_dir% /s /NFL /NDL /NJH /NJS

:: Delete compiled shaders from data
FOR /F "delims=" %%a IN ('dir "%input_dir%" /b /s') DO (
    IF /I "%%~xa"==".spv" (
        DEL "%%a"
    )
)