import { expandGlob } from "std/fs/mod.ts";

const walker = expandGlob("src/**/*.{cpp,hpp}", { caseInsensitive: true });

for await (const entry of walker) {
    console.log(entry.path);
    const process = Deno.run({
        cmd: ["clang-format", "-i", entry.path]
    });

    await process.status();
}
