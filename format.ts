import { expandGlob } from "https://deno.land/std@0.115.1/fs/mod.ts";

const walker = expandGlob("src/**/*.{cpp,hpp}", { caseInsensitive: true });

for await (const entry of walker) {
    console.log(entry.path);
    Deno.run({
        cmd: ["clang-format", "-i", entry.path]
    });
}
