import cmakeUserPresets from "./CMakeUserPresets.json" assert { type: "json" };

let index = 0;

for (const buildPreset of cmakeUserPresets?.buildPresets) {
    const { name, configurePreset } = buildPreset;

    {
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

    {
        const displayString = `${configurePreset} (build - task ${
            index + 1
        } of ${cmakeUserPresets.buildPresets.length})`;

        console.log("=".repeat(displayString.length));
        console.log(displayString);
        console.log("=".repeat(displayString.length));

        const process = Deno.run({
            cmd: ["cmake", "--build", "--preset", name],
        });

        const status = await process.status();

        if (!status.success) {
            Deno.exit(1);
        }
    }

    ++index;
}
