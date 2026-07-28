<?php

declare(strict_types=1);

enum BuildType: string
{
    case Debug = 'Debug';
    case Release = 'Release';
    case ReleaseWithDebugInfo = 'RelWithDebInfo';
}

enum PHPVersion: string
{
    case PHP83 = '8.3';
    case PHP84 = '8.4';
    case PHP85 = '8.5';
    case PHP86 = '8.6';
}

enum PHPTS: string
{
    case ZTS = 'zts';
    case NTS = 'nts';
}

enum Backend: string
{
    case ScyllaCpp  = 'scylla-cpp';
    case Cassandra  = 'cassandra';
    case ScyllaRust = 'scylla-rust';

    public function nameSuffix(): string
    {
        return match ($this) {
            self::ScyllaCpp  => '',
            self::Cassandra  => 'Cassandra',
            self::ScyllaRust => 'ScyllaRust',
        };
    }
}

function preset(
    string $name,
    Backend $backend,
    BuildType $buildType,
    PHPVersion $phpVersion,
    PHPTS $phpTS,
): array {
    $fullName = $name . 'PHP' . $phpVersion->value . strtoupper($phpTS->value) . $backend->nameSuffix();

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
            'PHP_SCYLLADB_BACKEND' => $backend->value,
            'PHP_VERSION_FOR_PHP_CONFIG' => $phpVersion->value,
            'PHP_SCYLLADB_STATIC' => 'ON',
            'PHP_THREAD_SAFE' => $phpTS === PHPTS::ZTS ? 'ON' : 'OFF',
        ],
    ];
}

function main()
{
    $presets = [];

    foreach (PHPVersion::cases() as $phpVersion) {
        foreach (BuildType::cases() as $buildType) {
            foreach (Backend::cases() as $backend) {
                foreach (PHPTS::cases() as $ts) {
                    $preset = preset($buildType->value, $backend, $buildType, $phpVersion, $ts);
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
