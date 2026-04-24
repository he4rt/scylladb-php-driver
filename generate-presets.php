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
    case PHP83 = '8.3';
    case PHP84 = '8.4';
    case PHP85 = '8.5';
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
    PHPVersion $phpVersion = PHPVersion::PHP85,
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
            "CMAKE_BUILD_TYPE" => $buildType->value,
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
    $useCassandra = [true, false];
    $phpVersions = PHPVersion::cases();
    $phpTS = PHPTS::cases();

    $presets = [];

    foreach ($phpVersions as $phpVersion) {
        foreach (BuildType::cases() as $buildType) {
            foreach ($useCassandra as $useCassandraValue) {
                foreach ($phpTS as $ts) {
                    $preset = preset($buildType->value, $useCassandraValue, $buildType, $phpVersion, $ts);
                    $presets[$preset['name']] = $preset;
                }
            }
        }
    }


    $cmakePresets = [
        'version' => 2,
        "configurePresets" => array_values($presets),
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
