<?php

declare(strict_types=1);

enum BuildType: string
{
    case Debug = 'Debug';
    case Release = 'Release';
    case ReleaseWithDebugInfo = 'RelWithDebugInfo';
}

enum PHPVersion: string
{
    case PHP81 = '8.1';
    case PHP82 = '8.2';
    case PHP83 = '8.3';
    case PHP84 = '8.4';
}

enum PHPTS: string
{
    case ZTS = 'zts';
    case NTS = 'nts';
}

function preset(
    string $name,
    bool $useCassandra = true,
    BuildType $buildType = BuildType::Debug,
    PHPVersion $phpVersion = PHPVersion::PHP84,
    PHPTS $phpTS = PHPTS::NTS,
): array {
    $fullName = $name  . 'PHP' . $phpVersion->value . strtoupper($phpTS->value) . ($useCassandra ? 'Cassandra' : '');

    return [
        "name" => $fullName,
        "displayName" => $fullName,
        "description" => "",
        "generator" => "Ninja",
        "binaryDir" => './out/' . $fullName,
        "cacheVariables" => [
            "CMAKE_BUILD_TYPE" => "Debug",
            "CMAKE_INSTALL_PREFIX" => './out/' . $fullName . '/install',
            "ENABLE_SANITIZERS" => $buildType === BuildType::Debug ? 'ON' : 'OFF',
            'SANITIZE_UNDEFINED' => $buildType === BuildType::Debug ? 'ON' : 'OFF',
            'SANITIZE_ADDRESS' => $buildType === BuildType::Debug ? 'ON' : 'OFF',
            'USE_LIBCASSANDRA' => $useCassandra ? 'ON' : 'OFF',
            'PHP_VERSION_FOR_PHP_CONFIG' => $phpVersion->value,
            'LINK_LIBUV_STATIC' => 'ON',
            'PHP_DRIVER_STATIC' => 'ON',
            'PHP_THREAD_SAFE' => $phpTS === PHPTS::ZTS ? 'ON' : 'OFF',
        ],
    ];
}

function main()
{
    $presetNames = ['Debug', 'Release', 'RelWithDebugInfo'];
    $useCassandra = [true, false];
    $buildTypes = BuildType::cases();
    $phpVersions = PHPVersion::cases();
    $phpTS = PHPTS::cases();

    $presets = [];

    foreach ($presetNames as $presetName) {
        foreach ($useCassandra as $useCassandraValue) {
            foreach ($buildTypes as $buildType) {
                foreach ($phpVersions as $phpVersion) {
                    foreach ($phpTS as $ts) {
                        $presets[] = preset($presetName, $useCassandraValue, $buildType, $phpVersion, $ts);
                    }
                }
            }
        }
    }


    $cmakePresets = [
        'version' => 2,
        "configurePresets" => $presets,
    ];

    $result = @file_put_contents(
        __DIR__ . DIRECTORY_SEPARATOR . 'CMakePresets.json',
        json_encode($cmakePresets, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES),
        LOCK_EX,
    );

    if ($result === false) {
        echo 'Failed to write CMakePresets.json';
        exit(1);
    }
}


main();
