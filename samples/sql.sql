-- Sample SQL to exercise the highlight rules

-- DDL
CREATE TEMPORARY TABLE IF NOT EXISTS public.users (
  user_id BIGSERIAL PRIMARY KEY,
  email VARCHAR(255) UNIQUE NOT NULL,
  profile JSONB,
  balance DECIMAL(12,2) DEFAULT 0.00,
  created_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMPTZ,
  CONSTRAINT email_chk CHECK (email LIKE '%@%')
);

-- Indexes
CREATE INDEX CONCURRENTLY IF NOT EXISTS users_email_idx ON public.users USING btree (email);
CREATE INDEX IF NOT EXISTS users_profile_gin_idx ON public.users USING gin (profile);

-- Insert & returning
INSERT INTO public.users (email, profile, balance)
VALUES
  ('alice@example.com', '{"plan":"pro","tags":["a","b"]}'::jsonb, 25.50),
  ('bob@example.com',   '{"plan":"free","tags":["c"]}'::jsonb,   0.00)
RETURNING user_id, email, profile;

-- Update with CASE and CAST
UPDATE public.users u
SET balance = balance + CAST(5 AS DECIMAL),
    updated_at = CURRENT_TIMESTAMP,
    profile = jsonb_set(profile, '{last_seen}', to_jsonb(CURRENT_TIMESTAMP)),
    email = CASE
              WHEN email LIKE '%@example.com' THEN replace(email, '@example.com', '@example.org')
              ELSE email
            END
WHERE u.balance >= 0
RETURNING user_id, email, balance;

-- Delete with USING
DELETE FROM public.users AS u
USING public.users AS t
WHERE u.user_id = t.user_id
  AND u.email LIKE 'bob@%';

-- Window, CTE, aggregates
WITH recent AS (
  SELECT *
  FROM public.users
  WHERE created_at > NOW() - INTERVAL '30 days'
)
SELECT
  user_id,
  email,
  balance,
  SUM(balance) OVER (PARTITION BY 1 ORDER BY created_at ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) AS running_total,
  ROW_NUMBER() OVER (ORDER BY created_at DESC) AS rn
FROM recent
ORDER BY created_at DESC
LIMIT 50 OFFSET 0;

-- Joins and JSON
CREATE TEMP TABLE events (
  event_id BIGSERIAL PRIMARY KEY,
  user_id BIGINT REFERENCES public.users(user_id),
  payload JSONB,
  created_at TIMESTAMPTZ DEFAULT NOW()
);

INSERT INTO events (user_id, payload) VALUES
  (1, '{"type":"login","ip":"127.0.0.1"}'),
  (1, '{"type":"purchase","amount":9.99,"items":[{"sku":"A1","qty":1}]}' ),
  (2, '{"type":"login","ip":"10.0.0.2"}');

SELECT u.email, e.payload->>'type' AS event_type, e.payload
FROM public.users u
LEFT JOIN events e ON e.user_id = u.user_id
WHERE e.payload ? 'type'
ORDER BY e.created_at DESC;

-- Transaction control
BEGIN;
  UPDATE public.users SET balance = balance - 5 WHERE email LIKE 'alice%';
  INSERT INTO events (user_id, payload)
  SELECT user_id, jsonb_build_object('type','adjust','delta',-5) FROM public.users WHERE email LIKE 'alice%';
COMMIT;
