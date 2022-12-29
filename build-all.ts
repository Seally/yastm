import * as flags from "std/flags/mod.ts";
import * as t from "./scripts/typanion.ts";

import cmakeUserPresets from "./CMakeUserPresets.json" assert { type: "json" };

const args = flags.parse(Deno.args, {
    alias: {
        help: "h",
    },
    boolean: [
        "help",
        "all",
        "list-only",
        "rebuild",
        "skip-configure",
        "skip-build",
        "clean-only",
    ],
});

function printHelp() {
    console.log([
        "OPTIONS",
        "--help, -h        Prints this help.",
        "--all             Build all presets in CMakeUserPresets.json. If this is not",
        "                  specified, all build presets that do not have COPY_BUILD",
        "                  enabled will be excluded",
        "--list-only       List the presets that the script will process given the",
        "                  current arguments (had this flag not been specified).",
        "--rebuild         Cleans and builds the project.",
        "--skip-configure  Skip CMake configure step.",
        "--skip-build      Skip building step.",
        "--clean-only      Only clean the targets. Prevents CMake configure.",
        "",
        "In case of conflicting arguments, the priority is in this order:",
        "",
        "    help > list-only > clean-only > skip-configure == skip-build > rebuild",
    ].join("\n"));
}

const verifyArgs = t.isPartial({
    help: t.isBoolean(),
    h: t.isBoolean(),
    all: t.isBoolean(),
    "list-only": t.isBoolean(),
    rebuild: t.isBoolean(),
    "skip-configure": t.isBoolean(),
    "skip-build": t.isBoolean(),
    "clean-only": t.isBoolean(),
});

if (!verifyArgs(args)) {
    printHelp();
    Deno.exit(1);
}

if (args.help) {
    printHelp();
    Deno.exit();
}

function getBuildPresets() {
    let presets = cmakeUserPresets?.buildPresets ?? [];

    if (!args.all) {
        presets = presets.filter((preset) => {
            const configurePresetName = preset.configurePreset;

            const configurePreset = cmakeUserPresets?.configurePresets.find(
                (preset) => {
                    return preset.name === configurePresetName;
                },
            );

            return !!configurePreset?.cacheVariables?.COPY_BUILD;
        });
    }

    return presets;
}

function printBanner(message: string, borderCharacter = "=") {
    if (borderCharacter.length > 1) {
        throw new Error(
            `borderCharacter "${borderCharacter}" is longer than one character`,
        );
    }

    const border = borderCharacter.repeat(message.length);

    console.log(border);
    console.log(message);
    console.log(border);
}

const buildPresets = getBuildPresets();

if (buildPresets.length <= 0) {
    console.log("No build presets available given the current arguments.");
} else if (args["list-only"]) {
    console.log(
        `The following ${buildPresets.length} presets will be processed:`,
    );

    for (let index = 0; index < buildPresets.length; ++index) {
        console.log(`[${index + 1}] ${buildPresets[index].name}`);
    }
} else {
    for (let index = 0; index < buildPresets.length; ++index) {
        const buildPreset = buildPresets[index];
        const { name, configurePreset } = buildPreset;

        if (args["clean-only"]) {
            printBanner(
                `${configurePreset} (clean - task ${
                    index + 1
                } of ${buildPresets.length})`,
            );

            const process = Deno.run({
                cmd: [
                    "cmake",
                    "--build",
                    "--target",
                    "clean",
                    "--preset",
                    name,
                ],
            });

            const status = await process.status();

            if (!status.success) {
                Deno.exit(1);
            }
        } else {
            if (!args["skip-configure"]) {
                printBanner(
                    `${configurePreset} (configure - task ${
                        index + 1
                    } of ${buildPresets.length})`,
                );

                const process = Deno.run({
                    cmd: ["cmake", "--preset", configurePreset],
                });

                const status = await process.status();

                if (!status.success) {
                    Deno.exit(1);
                }
            }

            if (!args["skip-build"]) {
                printBanner(
                    `${configurePreset} (build - task ${
                        index + 1
                    } of ${buildPresets.length})`,
                );

                const runArgs = ["cmake", "--build"];

                if (args["rebuild"]) {
                    runArgs.push("--clean-first");
                }

                runArgs.push("--preset", name);

                const process = Deno.run({ cmd: runArgs });
                const status = await process.status();

                if (!status.success) {
                    Deno.exit(1);
                }
            }
        }
    }
}
