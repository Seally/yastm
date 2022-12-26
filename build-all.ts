import * as flags from "std/flags/mod.ts";
import * as t from "./scripts/typanion.ts";

import cmakeUserPresets from "./CMakeUserPresets.json" assert { type: "json" };

const args = flags.parse(Deno.args, {
    alias: {
        help: "h",
    },
    boolean: ["help", "rebuild", "skip-configure", "skip-build", "clean-only"],
});

function printHelp() {
    console.log([
        "OPTIONS",
        "--help, -h        Prints this help.",
        "--rebuild         Cleans and builds the project.",
        "--skip-configure  Skip CMake configure step.",
        "--skip-build      Skip building step.",
        "--clean-only      Only clean the targets. Prevents CMake configure.",
        "",
        "In case of conflicting arguments, the priority is in this order:",
        "",
        "    clean-only > skip-configure == skip-build > rebuild",
    ].join("\n"));
}

const verifyArgs = t.isPartial({
    help: t.isBoolean(),
    h: t.isBoolean(),
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

let index = 0;

for (const buildPreset of cmakeUserPresets?.buildPresets) {
    const { name, configurePreset } = buildPreset;

    if (args["clean-only"]) {
        const displayString = `${configurePreset} (clean - task ${
            index + 1
        } of ${cmakeUserPresets.buildPresets.length})`;

        console.log("=".repeat(displayString.length));
        console.log(displayString);
        console.log("=".repeat(displayString.length));

        const process = Deno.run({
            cmd: ["cmake", "--build", "--target", "clean", "--preset", name],
        });

        const status = await process.status();

        if (!status.success) {
            Deno.exit(1);
        }
    } else {
        if (!args["skip-configure"]) {
            const displayString = `${configurePreset} (configure - task ${
                index + 1
            } of ${cmakeUserPresets.buildPresets.length})`;

            console.log("=".repeat(displayString.length));
            console.log(displayString);
            console.log("=".repeat(displayString.length));

            const process = Deno.run({
                cmd: ["cmake", "--preset", configurePreset],
            });

            const status = await process.status();

            if (!status.success) {
                Deno.exit(1);
            }
        }

        if (!args["skip-build"]) {
            const displayString = `${configurePreset} (build - task ${
                index + 1
            } of ${cmakeUserPresets.buildPresets.length})`;

            console.log("=".repeat(displayString.length));
            console.log(displayString);
            console.log("=".repeat(displayString.length));

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

    ++index;
}
